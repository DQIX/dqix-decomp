#include "Filesystem/FSInnerDefs.h"
#include "System/BiosData.h"
#include "System/Memory.h"
#include "System/Interrupts.h"
#include "System/Cache.h"
#include "System/DMA.h"
#include <globaldefs.h>
#include <asmhacks.h>

#pragma optimize_for_size off

#define ADDR_ITCM_START 0x01ff8000
#define ADDR_ITCM_END 0x02000000

#define ADDR_GAMECARD_BUS_ROMCTRL 0x040001a4
#define ADDR_GAMECARD_RECEIVED_DATA 0x04100010
#define GAMECARD_BUS_COMMAND_BYTE(n) *(unsigned char*)(0x040001a8 + n)

#define ROMCTRL_FLAG_WORD_READY (1u << 23)
#define ROMCTRL_FLAG_BUSY (1u << 31)

#if defined(jpn)
#define func_020c89e4 func_020ca4b0
#define func_020ca8e8 func_020cc3b4
#define func_020d1118 func_020d2be4
#define func_020d1234 func_020d2d00
#endif

void SendTaskToReadContext(CardReadManager::ReadProc);

extern "C"
{
    // Gets the base of the tightly coupled memory region
    unsigned int func_020c89e4();

    // Seems to set up DMA to repeatedly read from the read parameter into write
    // pointer, incrementing the write pointer but not the read pointer.
    void func_020ca8e8(unsigned int dmaChannel, const void* readFrom, void* writeTo, unsigned int length);
    
    void func_020d1118();

    void func_020d1234(unsigned int);
}

void WaitForReadManagerIdle_Internal();
Struct_02111f20::LowLevelReadProc GetLowLevelCartridgeReadProc();

// Returns true if there's more to be copied after
extern "C" bool TransferScratchBuffer(Struct_02111f20* input)
{
    CardReadManager* ptr = &data_021118e0;
    unsigned int romCurrentBlockStart = ptr->cartridgeReadOffset & 0xfffffe00;
    if (romCurrentBlockStart == (unsigned int)input->alignedWrite)
    {
        unsigned int offsetFromAlignment = ptr->cartridgeReadOffset
            - romCurrentBlockStart;
        unsigned int usefulLength = 0x200 - offsetFromAlignment;
        if (usefulLength > ptr->writeLength)
            usefulLength = ptr->writeLength;
        
        VectorizedInvertedMemcpy(input->scratchBuffer + offsetFromAlignment,
            ptr->writeDst, usefulLength);
        ptr->cartridgeReadOffset += usefulLength;
        ptr->writeDst += usefulLength;
        ptr->writeLength -= usefulLength;
    }
    return ptr->writeLength != 0;
}

// Notably this is not a uint64 parameter. This way you can write
// (0x11223344, 0x55667788) and it will write the eight bytes
// (11, 22, 33, 44, 55, 66, 77, 88)
void SendGamecardBusCommand(unsigned int firstWord, unsigned int secondWord)
{
    volatile unsigned int* control = (unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL;
    while (*control & ROMCTRL_FLAG_BUSY) {}

    // Upper byte of "AUXSPICNT - Gamecard ROM and SPI Control (R/W)"
    // Set byte 15 (enable NDS slot), set byte 14 (enable transfer ready IRQ)
    // and clear byte 13 (0 meaning parallel NDS slot mode)
    // (Note bytes 12-8 are unused)
    *(unsigned char*)0x40001a1 = 0xC0;

    GAMECARD_BUS_COMMAND_BYTE(0) = firstWord >> 24;
    GAMECARD_BUS_COMMAND_BYTE(1) = firstWord >> 16;
    GAMECARD_BUS_COMMAND_BYTE(2) = firstWord >> 8;
    GAMECARD_BUS_COMMAND_BYTE(3) = firstWord >> 0;

    GAMECARD_BUS_COMMAND_BYTE(4) = secondWord >> 24;
    GAMECARD_BUS_COMMAND_BYTE(5) = secondWord >> 16;
    GAMECARD_BUS_COMMAND_BYTE(6) = secondWord >> 8;
    GAMECARD_BUS_COMMAND_BYTE(7) = secondWord >> 0;
}

void ReadSingleSegmentFromCartridge()
{
    CardReadManager* readManager = &data_021118e0;
    void* gamecardReceivedDataRegister = (void*)ADDR_GAMECARD_RECEIVED_DATA;
    Struct_02111f20* ps1f20 = &data_02111f00.innerStruct;

    DECLARE_ASM_NOP();

    func_020ca8e8(readManager->dmaChannel, gamecardReceivedDataRegister, readManager->writeDst, 0x200);
    unsigned int offset = readManager->cartridgeReadOffset;
    SendGamecardBusCommand(0xb7000000 | (offset >> 8), offset << 24);
    *(unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL = ps1f20->control_4;
}

void DMAChainSegmentInterruptHandler()
{
    ResetDMAChannel(data_021118e0.dmaChannel);

    CardReadManager* manager = &data_021118e0;
    
    manager->cartridgeReadOffset += 512;
    manager->writeDst += 512;
    manager->writeLength -= 512;

    bool moreToWrite = manager->writeLength != 0;

    if (!moreToWrite)
    {
        DisableSpecificInterrupts(IRQ_MASK_GAMECARD_DATA_TRANSFER_DONE);
        AcknowledgeSpecificInterrupts(IRQ_MASK_GAMECARD_DATA_TRANSFER_DONE);
        CardReadManager* readManager = &data_021118e0;
        unsigned int romChipID = SetupNormalGamecardBusCommandMode();
        func_020d1234(romChipID);

        readManager->pSharedData->unknown_0 = 0;
        CardReadManager::CompletionCallback cleanProc = readManager->onComplete;
        NitroHandle* handle = readManager->handle;

        int oldState = DisableIRQInterrupts();
        readManager->flags &= ~((1 << READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS) | (1 << READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING) | (1 << CARTRIDGE_READ_CONTEXT_FLAG_6));
        UnblockContexts(&readManager->ongoingReadBlock);

        if (readManager->flags & (1 << CARTRIDGE_READ_CONTEXT_FLAG_4))
            MarkContextReadyAndSwitch(&readManager->cartridgeReadContext);
        SetIRQInterruptState(oldState);

        if (cleanProc != NULL)
            cleanProc(handle);
    }
    else
    {
        DECLARE_ASM_NOP();
        ReadSingleSegmentFromCartridge();
    }
}

extern "C" bool TryReadViaDMA(Struct_02111f20* handler)
{
    CardReadManager* readManager = &data_021118e0;
    unsigned int length = readManager->writeLength;
    
    bool canDoDMARead = false;
    bool readAlignedToCartridgeSegments = false;
    bool destinationIsSuitableMemoryType = false; // i.e. not DTCM or ITCM
    bool destinationIsCacheAligned = false; 
    
    unsigned int destination = (unsigned int)readManager->writeDst;
    unsigned int alignmentMod32 = (unsigned int)destination & 0x1f;
    
    if (alignmentMod32 == 0 && ((readManager->dmaChannel <= 3)))
        destinationIsCacheAligned = true;
    
    if (destinationIsCacheAligned)
    {
        unsigned int dtcmBase = func_020c89e4();
        bool isDTCM = true;
        bool isITCM = false;
        if (destination + length > ADDR_ITCM_START && destination < ADDR_ITCM_END)
            isITCM = true;
        
        // 0x4000 = 16kb is the size of data tightly coupled memory
        if (!isITCM && (dtcmBase >= destination + length || dtcmBase + 0x4000 <= destination))
            isDTCM = false;
        
        if (!isDTCM)
            destinationIsSuitableMemoryType = true;
    }

    if (destinationIsSuitableMemoryType && ((readManager->cartridgeReadOffset | length) & 0x1ff) == 0)
        readAlignedToCartridgeSegments = true;

    if (readAlignedToCartridgeSegments && length != 0)
        canDoDMARead = true;
    
    handler->control_4 = (data_020f22cc->gamecardBusControlNormalSettings & ~0x07000000) | 0xa1000000;
    
    if (canDoDMARead)
    {
        int oldState = DisableIRQInterrupts();
        if (length < readManager->instructionCacheCleanThreshold)
            InvalidateInstructionCacheRange((void*)destination, length);
        else
            InvalidateInstructionCache();

        if (length < readManager->dataCacheCleanThreshold)
        {
            if (alignmentMod32 != 0)
            {
                destination -= alignmentMod32;
                CleanCacheRange((void*)destination, 0x20);
                CleanCacheRange((void*)(destination + length), 0x20);
                length += 0x20;
            }
            InvalidateDataCacheRange((void*)destination, length);
            DrainWriteBuffer();
        }
        else
        {
            CleanInvalidateDataCache();
        }

        SetInterruptHandler(IRQ_MASK_GAMECARD_DATA_TRANSFER_DONE, &DMAChainSegmentInterruptHandler);
        AcknowledgeSpecificInterrupts(IRQ_MASK_GAMECARD_DATA_TRANSFER_DONE);
        EnableSpecificInterrupts(IRQ_MASK_GAMECARD_DATA_TRANSFER_DONE);
        SetIRQInterruptState(oldState);
        ReadSingleSegmentFromCartridge();
    }

    return canDoDMARead;
}

extern "C" void ReadBlocksFromCartridge(Struct_02111f20* handler)
{
    CardReadManager* readManager = &data_021118e0;
    while (true)
    {
        unsigned int alignedOffset = readManager->cartridgeReadOffset & 0xfffffe00;
        unsigned int* practicalWriteLoc;
        // If read offset is not 512-byte aligned, write dst is not 4-byte aligned
        // or we're loading < 512 bytes, use the scratch space in the input struct
        if (alignedOffset != readManager->cartridgeReadOffset ||
            ((unsigned int)readManager->writeDst & 3) ||
            readManager->writeLength < 0x200)
        {
            handler->alignedWrite = (unsigned int*)alignedOffset;
            practicalWriteLoc = (unsigned int*)handler->scratchBuffer;
        }
        else
            practicalWriteLoc = (unsigned int*)readManager->writeDst;
        SendGamecardBusCommand(0xb7000000 | (alignedOffset >> 8), alignedOffset << 24);
        *(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL = handler->control_4;

        unsigned int control;
        int wordIndex = 0;
        unsigned int bytesCopied = 0;
        do {
            control = *(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL;
            if (control & ROMCTRL_FLAG_WORD_READY)
            {
                unsigned int data = *(volatile unsigned int*)ADDR_GAMECARD_RECEIVED_DATA;
                // The gamecard bus command only allows for reading 512 bytes,
                // so is this necessary? I guess if you try to read past the end
                // and get garbage data?
                if (bytesCopied < 0x200)
                {
                    practicalWriteLoc[wordIndex] = data;
                    bytesCopied += 4;
                    wordIndex++;
                }
            }
        } while (control & ROMCTRL_FLAG_BUSY);

        // If we performed an aligned write, can just move on to the next
        // aligned write
        if (practicalWriteLoc == (unsigned int*)readManager->writeDst)
        {
            unsigned int remainingLength;
            volatile CardReadManager* manager = &data_021118e0;
            manager->cartridgeReadOffset += 0x200;
            manager->writeDst += 0x200;
            remainingLength = manager->writeLength - 0x200;
            manager->writeLength = remainingLength;
            if (remainingLength != 0)
                continue;
            break;
        }
        else if (!TransferScratchBuffer(handler))
            break;
    }
}

unsigned int SetupNormalGamecardBusCommandMode()
{
    // Get ROM chip ID
    SendGamecardBusCommand(0xb8000000, 0x00000000);
    unsigned int newSettings = data_020f22cc->gamecardBusControlNormalSettings;
    newSettings &= ~0x07000000; // clear data block size to 0 = none
    newSettings |=  0xa7000000; // set block size to 7 = 4 bytes, block status = started/busy, release RESB?
    newSettings &=  0xffffe000; // disable bit 14 (unknown?) and 13 (don't KEY2 encrypt)
    *(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL = newSettings;
    do {
    } while ((*(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL & ROMCTRL_FLAG_WORD_READY) == 0);
    return *(volatile unsigned int*)ADDR_GAMECARD_RECEIVED_DATA;
}

// I don't know that this is safe per se, but in practice it wraps
// ReadBlocksFromCartridge (the readProc) with some auxiliary stuff
extern "C" void SafeReadBlocksFromCartridge(CardReadManager*)
{
    Struct_02111f20* rawHandler = &data_02111f00.innerStruct;
    if (TransferScratchBuffer(rawHandler))
    {
        Struct_02111f20::LowLevelReadProc readProc = rawHandler->lowLevelReadProc;
        // This is (usually?) ReadBlocksFromCartridge
        readProc(rawHandler);
    }

    CardReadManager* readManager = &data_021118e0;
    unsigned int romChipID = SetupNormalGamecardBusCommandMode();
    // Does a few things, but in the end it resets all four DMA channels,
    // so it's some kind of cleanup
    func_020d1234(romChipID);

    readManager->pSharedData->unknown_0 = 0;
    CardReadManager::CompletionCallback callback = readManager->onComplete;
    NitroHandle* handle = readManager->handle;

    int oldState = DisableIRQInterrupts();
    readManager->flags &= ~((1 << READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS) | (1 << READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING) | (1 << CARTRIDGE_READ_CONTEXT_FLAG_6));
    UnblockContexts(&readManager->ongoingReadBlock);

    if (readManager->flags & (1 << CARTRIDGE_READ_CONTEXT_FLAG_4))
        MarkContextReadyAndSwitch(&readManager->cartridgeReadContext);
    SetIRQInterruptState(oldState);
    
    if (callback != NULL)
        callback(handle);

    DECLARE_ASM_NOP();
}

void LoadDataFromCartridgeToMemory(unsigned int dmaChannel,
    unsigned int cartridgeOffset, void* dest, unsigned int length,
    CardReadManager::CompletionCallback onComplete, NitroHandle* handle, CBool async)
{
    Struct_02111f20* rawHandler = &data_02111f00.innerStruct;
    CardReadManager* readManager = &data_021118e0;
    VerifyCardReadManagerInitialized();

    int oldState = DisableIRQInterrupts();

    while (readManager->flags & (1 << READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS))
    {
        BlockCurrentContext(&readManager->ongoingReadBlock);
    }
    readManager->flags |= (1 << READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS);
    readManager->onComplete = onComplete;
    readManager->handle = handle;
    SetIRQInterruptState(oldState);
    readManager->cartridgeReadOffset = cartridgeOffset + data_02111f00.number;
    readManager->dmaChannel = dmaChannel;
    readManager->writeDst = (unsigned char*)dest;
    readManager->writeLength = length;

    if (dmaChannel <= 3)
        ResetDMAChannel(dmaChannel);

    if (TryReadViaDMA(rawHandler))
    {
        if (!async)
            WaitForReadManagerIdle_Internal();
    }
    else if (async)
    {
        SendTaskToReadContext(&SafeReadBlocksFromCartridge);
    }
    else
    {
        readManager->currentTaskExecutionContext = data_02111304.activeContext;
        SafeReadBlocksFromCartridge(readManager);
    }
}

void InitializeCardReading()
{
    CardReadManager* readManager = &data_021118e0;
    if (readManager->flags != 0)
        return;

    readManager->flags = (1 << CARTRIDGE_READ_CONTEXT_FLAG_0);
    readManager->writeLength = 0;
    readManager->writeDst = NULL;
    readManager->cartridgeReadOffset = 0;
    readManager->dmaChannel = -1;
    readManager->onComplete = NULL;
    readManager->handle = NULL;

    data_02111f00.number = 0;
    InitializeCardReadManager();
    data_02111f00.innerStruct.lowLevelReadProc = GetLowLevelCartridgeReadProc();
    func_020d1118();
}

void WaitForReadManagerIdle_Internal()
{
    AwaitCardReadManagerIdle();
}

Struct_02111f20::LowLevelReadProc GetLowLevelCartridgeReadProc()
{
    return &ReadBlocksFromCartridge;
}

void IPCCommand11Proc(unsigned int command, unsigned int argument, unsigned int flag)
{
    if (command != 11 || !flag)
        return;

    CardReadManager* manager = (CardReadManager*)&data_021118e0;
    
    ProcessorContext* context = manager->currentTaskExecutionContext;
    manager->flags &= ~(1 << READ_MANAGER_FLAG_AWAITING_ARM7_ACTION);
    
    MarkContextReadyAndSwitch(context);
}

void CartridgeReadContextLoop()
{
    CardReadManager* readManager = &data_021118e0;
    while (true)
    {
        int priorState = DisableIRQInterrupts();
        if (!(readManager->flags & (1 << READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING)))
        {
            do {
                BlockCurrentContext(NULL);
            } while (!(readManager->flags & (1 << READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING)));
        }
        SetIRQInterruptState(priorState);
        readManager->cartridgeReadProc(readManager);
    }
}