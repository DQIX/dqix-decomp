#include "Filesystem/FSInnerDefs.h"
#include "Filesystem/CardReadManager.h"
#include "System/Interrupts.h"
#include "System/BiosData.h"
#include "System/GamecardBusOwnership.h"
#include <globaldefs.h>

#pragma optimize_for_size off

#if defined(jpn)
#define data_020f228c data_020f23f8
#endif

extern char data_020f228c[]; // "rom"

CBool CreateFileAccessor(NitroFileAccessor* outAccessor, const char* path)
{
    NitroVM machine;
    NitroVM_Initialize(&machine);

    return NitroVM_SearchFileOrDirectory(&machine, path, outAccessor, NULL) != false;
}

CBool NitroVM_PrepareRead(NitroVM* vm, NitroHandle* handle,
    unsigned int start, unsigned int end, unsigned int fileID)
{
    vm->linkedHandle = handle;
    vm->args_CopyRegisters.fileID = fileID;
    vm->args_CopyRegisters.startOffset = start;
    vm->args_CopyRegisters.endOffset = end;

    // Operand 7 copies capacity into base_a,
    // start into base_b and base_d, end into base_c.
    if (!NitroVM_QueueCommand(vm, NITROVM_OPCODE_COPY_REGISTERS))
        return false;

    vm->flags = (vm->flags | (1 << NITROVM_FLAG_READ_POSITIONS_CONFIGURED)) & ~(1 << NITROVM_FLAG_SEARCH_TARGET_IS_DIRECTORY);
    return true;
}

// Accessor passed by value (in registers r1, r2)
bool NitroVM_PrepareReadFileByID(NitroVM* vm, NitroFileAccessor volatile accessor)
{
    NitroHandle* handle = accessor.handle;
    if (handle == NULL)
        return false;

    vm->linkedHandle = handle;
    vm->args_GetFATEntry.accessor.handle = handle;
    vm->args_GetFATEntry.accessor.fileID = accessor.fileID;
    if (!NitroVM_QueueCommand(vm, NITROVM_OPCODE_GET_FAT_ENTRY))
        return false;

    vm->flags = (vm->flags | (1 << NITROVM_FLAG_READ_POSITIONS_CONFIGURED)) & ~(1 << NITROVM_FLAG_SEARCH_TARGET_IS_DIRECTORY);
    return true;
}

bool NitroVM_PrepareReadFileByPath(NitroVM* vm, const char* path)
{
    NitroFileAccessor accessor;
    if (CreateFileAccessor(&accessor, path))
    {
        if (NitroVM_PrepareReadFileByID(vm, accessor))
            return true;
    }

    return false;
}

CBool NitroVM_FinishRead(NitroVM* vm)
{
    // this opcode executes synchronously, so when control returns here all
    // VMs before it will have executed their instructions
    if (!NitroVM_QueueCommand(vm, NITROVM_OPCODE_NOP))
        return false;

    vm->linkedHandle = NULL;
    vm->pendingCommand = NITROVM_OPCODE_MAYBE_INVALID;
    vm->flags &= ~((1 << NITROVM_FLAG_READ_POSITIONS_CONFIGURED) | (1 << NITROVM_FLAG_SEARCH_TARGET_IS_DIRECTORY));
    return true;
}

void NitroVM_WriteOutFilePath(NitroVM* vm, char* buffer, unsigned int bufferLength)
{
    if (vm->pendingCommand != NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_PATH)
    {
        vm->args_GetPath.numBytesWritten = 0;
        vm->args_GetPath.directoryID = 0;
    }

    vm->args_GetPath.pathOutput = buffer;
    vm->args_GetPath.outputCapacity = bufferLength;

    // This might be a bool function, in which case we return the return value
    // of this. (We still get tail call optimisation though)
    NitroVM_QueueCommand(vm, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_PATH);
}

CBool NitroVM_AwaitCommandCompletion(NitroVM* vm)
{
    CBool executionPending = false;

    int oldState = DisableIRQInterrupts();

    if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_IN_HANDLE_QUEUE))
    {
        // Execute iff flag 6 and flag 2 are both clear.
        executionPending = !(vm->flags & ((1 << NITROVM_FLAG_READY_TO_EXECUTE) | (1 << NITROVM_FLAG_SYNCHRONOUS)));

        if (executionPending)
        {
            vm->flags |= (1 << NITROVM_FLAG_SYNCHRONOUS);
            do {
                BlockCurrentContext(&vm->blockedContexts);
            } while (!(vm->flags & (1 << NITROVM_FLAG_READY_TO_EXECUTE))); // why the different format here?
        }
        else
        {
            do {
                BlockCurrentContext(&vm->blockedContexts);
            } while (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_IN_HANDLE_QUEUE));
        }
    }

    SetIRQInterruptState(oldState);

    if (executionPending)
        return NitroVM_ExecuteAndUnlink(vm);
    else
        return vm->storedResult == NITRO_RESULT_SUCCESS;
}

void NitroVM_CancelCommand(NitroVM* vm)
{
    int oldState = DisableIRQInterrupts();

    if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_IN_HANDLE_QUEUE))
    {
        // mark for removal from the handle's list of attached VMs
        vm->flags |= (1 << NITROVM_FLAG_MARKED_FOR_UNLINK_FROM_HANDLE);
        vm->linkedHandle->flags |= (1 << NITROHANDLE_FLAG_VM_LIST_DIRTY);
    }
    SetIRQInterruptState(oldState);
}

int NitroVM_ReadAsync(NitroVM* vm, void* dst, int capacity)
{
    return NitroVM_Read(vm, dst, capacity, true);
}

int NitroVM_ReadSync(NitroVM* vm, void* dst, int capacity)
{
    return NitroVM_Read(vm, dst, capacity, false);
}

CBool NitroVM_Seek(NitroVM* vm, int offset, int whence)
{
    switch (whence)
    {
    case 0: // SEEK_SET
        offset += vm->fileInfo.startOffset;
        break;
    case 1: // SEEK_CUR
        offset += vm->fileInfo.cursorPos;
        break;
    case 2: // SEEK_END
        offset += vm->fileInfo.endOffset;
        break;
    default:
        return false;
    }
        
    if (offset < vm->fileInfo.startOffset)
        offset = vm->fileInfo.startOffset;
    if (offset > vm->fileInfo.endOffset)
        offset = vm->fileInfo.endOffset;

    vm->fileInfo.cursorPos = offset;
    return true;
}

CBool SetROMFilesystemRoot(const char* path)
{
    NitroVM vm;
    NitroVM_Initialize(&vm);
    NitroDirectoryAccessor accessor;
    if (!NitroVM_SearchFileOrDirectory(&vm, path, NULL, &accessor))
        return false;

    data_02111728.romFSRoot = accessor;
    return true;
}

void OnCartridgeDataLoadCompletion(NitroHandle* handle)
{
    int result = func_020d1198() ? NITRO_RESULT_5 : NITRO_RESULT_SUCCESS;
    NitroHandle_OnTaskCompletion(handle, result);
}

int ROMFilesystemLoadProc(NitroHandle* handle, void* dst, unsigned offset, unsigned len)
{
    LoadDataFromCartridgeToMemory(data_0211173c.dmaChannel, offset, dst, len, &OnCartridgeDataLoadCompletion, handle, true);
    return NITRO_RESULT_TASK_STILL_RUNNING;
}

int ROMFilesystemSaveProc(NitroHandle*, const void*, unsigned, unsigned)
{
    return NITRO_RESULT_FAILURE;
}

int ROMFilesystemOpcodeOverride(NitroVM* vm, int opcode)
{
    if (opcode != NITROVM_OPCODE_SAVE)
    {
        switch (opcode)
        {
        case NITROVM_OPCODE_ACQUIRE_NDS_BUS:
            NitroVM_Command_AcquireCardReadResources(data_0211173c.busHolderID);
            return NITRO_RESULT_SUCCESS;
        case NITROVM_OPCODE_RELEASE_NDS_BUS:
            NitroVM_Command_ReleaseCardReadResources(data_0211173c.busHolderID);
            return NITRO_RESULT_SUCCESS;
        }
    }
    else // opcode = 1 (save), not possible for ROM data
        return NITRO_RESULT_COMMAND_UNSUPPORTED;

    // Override does not implement command
    return NITRO_RESULT_OPCODE_NOT_IMPLEMENTED;
}

int UnsupportedFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned)
{
    return NITRO_RESULT_FAILURE;
}

int UnsupportedFilesystemOpcodeOverride(NitroVM*, int)
{
    return NITRO_RESULT_COMMAND_UNSUPPORTED;
}

void InitializeROMFilesystem_Internal(unsigned int channel)
{
    data_0211173c.dmaChannel = channel;
    data_0211173c.busHolderID = GenerateLockOwnerID();
    data_0211173c.arm9Data.start = NULL;
    data_0211173c.arm9Data.size = 0;
    data_0211173c.arm7Data.start = NULL;
    data_0211173c.arm7Data.size = 0;
    InitializeCardReading();

    NitroHandle_Initialize(&data_02111754);
    NitroHandle_AddToHandleList(&data_02111754, data_020f228c, 3);

    const BootIndicator* biosData = (BootIndicator*)BIOS_ADDR_BOOT_INDICATOR;
    const CartridgeHeader& cartridgeHeader = biosData->cartHeader;
    if (IsDownloadPlay()) // is this even a thing for DQ9?
    {
        data_0211173c.arm9Data.start = (void*)-1;
        data_0211173c.arm9Data.size = 0;
        data_0211173c.arm7Data.start = (void*)-1;
        data_0211173c.arm7Data.size = 0;
        NitroHandle_SetOpcodeOverride(&data_02111754, &UnsupportedFilesystemOpcodeOverride, -1);
        NitroHandle_Populate(&data_02111754, NULL, 0, 0, 0, 0,
            &UnsupportedFilesystemLoadProc, &ROMFilesystemSaveProc);
        return;
    }

    NitroHandle_SetOpcodeOverride(&data_02111754, &ROMFilesystemOpcodeOverride,
        (1 << NITROVM_OPCODE_RELEASE_NDS_BUS) | (1 << NITROVM_OPCODE_ACQUIRE_NDS_BUS) | (1 << NITROVM_OPCODE_SAVE));

    if (cartridgeHeader.fileNameTableOffset == -1 ||
        cartridgeHeader.fileNameTableOffset == 0 ||
        cartridgeHeader.fileAllocTableOffset == -1 ||
        cartridgeHeader.fileAllocTableOffset == 0)
        return;
    
    NitroHandle_Populate(&data_02111754, NULL, 
        cartridgeHeader.fileAllocTableOffset,
        cartridgeHeader.fileAllocTableSize,
        cartridgeHeader.fileNameTableOffset,
        cartridgeHeader.fileNameTableSize,
        &ROMFilesystemLoadProc,
        &ROMFilesystemSaveProc);
}

unsigned int ChangeROMLoadDMAChannel(unsigned int newChannel)
{
    int oldState = DisableIRQInterrupts();
    unsigned int oldChannel = data_0211173c.dmaChannel;
    CBool destructionResult = NitroHandle_Pause(&data_02111754);
    data_0211173c.dmaChannel = newChannel;
    if (destructionResult)
        NitroHandle_Unpause(&data_02111754);
    SetIRQInterruptState(oldState);
    return oldChannel;
}

unsigned int LoadROMFilesystemFileTables(void* where, unsigned int capacity)
{
    return NitroHandle_LoadFileTables(&data_02111754, where, capacity);
}