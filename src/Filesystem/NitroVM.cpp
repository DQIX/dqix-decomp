#include "Filesystem/FSInnerDefs.h"
#include "System/Interrupts.h"
#include "System/Memory.h"
#include <globaldefs.h>
#include <asmhacks.h>

// -O2,p optimization seems to be needed here
#pragma optimize_for_size off

typedef int(*FixedCommand)(NitroVM*);

#if defined(jpn)
#define data_020f2288 data_020f23f4
#endif

#define NITROFS_ID_INVALID 0x10000
extern unsigned char data_020f2288[]; // ":/"

extern "C" void NitroVM_UnlinkAndStoreResult(NitroVM* vm, int result)
{
    int priorIRQState = DisableIRQInterrupts();

    NitroVM* prev = vm->links.pPrev;
    NitroVM* next = vm->links.pNext;

    if (prev != NULL)
        prev->links.pNext = next;
    
    if (next != NULL)
        next->links.pPrev = prev;

    vm->links.pPrev = NULL;
    vm->links.pNext = NULL;
    // Clear flags 0, 1, 2, 3 and 6
    vm->flags &= ~(
        (1 << NITROVM_FLAG_IN_HANDLE_QUEUE) |
        (1 << NITROVM_FLAG_MARKED_FOR_UNLINK_FROM_HANDLE) |
        (1 << NITROVM_FLAG_SYNCHRONOUS) |
        (1 << NITROVM_FLAG_EXECUTING_FROM_QUEUE) |
        (1 << NITROVM_FLAG_READY_TO_EXECUTE));
    vm->storedResult = result;

    UnblockContexts(&vm->blockedContexts);

    SetIRQInterruptState(priorIRQState);
}

extern "C" int NitroVM_ExecuteCommand(NitroVM* vm, int opcode)
{
    int result;
    int machineStartingFlags = vm->flags;
    NitroHandle* nitroHandle = vm->linkedHandle;
    int opcodeMask = 1 << opcode;

    // Bit verbose, but this way works with volatile and non-volatile
    unsigned int handleFlagAdjust = nitroHandle->flags;
    if (GET_FLAG_BIT(machineStartingFlags, NITROVM_FLAG_SYNCHRONOUS))
        handleFlagAdjust |= (1 << NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS);
    else
        handleFlagAdjust |= (1 << NITROHANDLE_FLAG_ASYNC_COMMAND_IN_PROGRESS);
    nitroHandle->flags = handleFlagAdjust;

    if ((nitroHandle->overrideOpcodeFlags & opcodeMask))
    {
        result = nitroHandle->instructionOverride(vm, opcode);
        switch (result)
        {
        case NITRO_RESULT_SUCCESS:
        case NITRO_RESULT_FAILURE:
        case NITRO_RESULT_COMMAND_UNSUPPORTED:
            vm->storedResult = result;
            break;
        case NITRO_RESULT_OPCODE_NOT_IMPLEMENTED:
            nitroHandle->overrideOpcodeFlags &= ~opcodeMask;
            result = NITRO_RESULT_FALLBACK_TO_DEFAULT;
            break;
        }
    }
    else
        result = NITRO_RESULT_FALLBACK_TO_DEFAULT;

    if (result == NITRO_RESULT_FALLBACK_TO_DEFAULT)
    {
        static const FixedCommand defaultCommands[] = {
            &NitroVM_DefaultCommand_Read,
            &NitroVM_DefaultCommand_Write,
            &NitroVM_DefaultCommand_GetDirectoryData,
            &NitroVM_DefaultCommand_GetFileOrDirectoryNameData,
            &NitroVM_DefaultCommand_GetFileOrDirectoryByName,
            &NitroVM_DefaultCommand_GetPath,
            &NitroVM_DefaultCommand_GetFATEntry,
            &NitroVM_DefaultCommand_CopyRegisters,
            &NitroVM_DefaultCommand_Nop
        };
        result = defaultCommands[opcode](vm);
    }

    if (result == NITRO_RESULT_TASK_STILL_RUNNING)
    {
        if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SYNCHRONOUS))
        {
            int priorIRQState = DisableIRQInterrupts();
            if (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS))
            {
                do
                {
                    BlockCurrentContext(&nitroHandle->taskWaitBlock);
                } while (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS));
            }
            result = vm->storedResult;
            SetIRQInterruptState(priorIRQState);
        }
    }
    else
    {
        if (!GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SYNCHRONOUS))
        {
            nitroHandle->flags &= ~(1 << NITROHANDLE_FLAG_ASYNC_COMMAND_IN_PROGRESS);
            NitroVM_UnlinkAndStoreResult(vm, result);
        }
        else
        {
            nitroHandle->flags &= ~(1 << NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS);
            vm->storedResult = result;
        }
    }
    
    return result;
}

extern "C" int CaseInsensitiveStrncmp(const unsigned char* first,
    const unsigned char* second, unsigned int len)
{
    unsigned int index = 0;
    // needed to prevent optimizing to len != 0
    unsigned int ZERO = 0;
    if (len > ZERO)
    {
        do
        {
            unsigned int charA = first[index] - 'A';
            unsigned int charB = second[index] - 'A';

            if (charA <= 25)
                charA += 'a' - 'A';

            if (charB <= 25)
                charB += 'a' - 'A';

            if (charA != charB)
                return charA - charB;
            index++;
        } while (index < len);
    }
    return 0;
}

int Nitro_ReadMetadataBytes(FSReadDescription* readDesc, void* dst, unsigned int len)
{
    NitroHandle* nitroHandle = readDesc->nitroHandle;
    nitroHandle->flags |= (1 << NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS);

    int result = nitroHandle->fastReadProc(nitroHandle, dst, readDesc->offset, len);
    switch (result)
    {
    case NITRO_RESULT_SUCCESS:
    case NITRO_RESULT_FAILURE:
        nitroHandle->flags &= ~(1 << NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS);
        break;
    case NITRO_RESULT_TASK_STILL_RUNNING:
    {
        int priorState = DisableIRQInterrupts();
        if (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS))
        {
            do
            {
                BlockCurrentContext(&nitroHandle->taskWaitBlock);
            } while (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS));
        }

        SetIRQInterruptState(priorState);
        result = nitroHandle->linkToFirstVM.pNext->storedResult;
    }
    }
    readDesc->offset += len;
    return result;
}

// Sets up and executes command 2 with 'proper' registers (i.e. stuff that
// needs to be zero for proper behaviour is set to zero). This means, afterwards
// base_b_low holds the id of this directory (i.e. input parameter)
// base_b_high holds the id of the first file in the directory
// base_c holds an offset to the relevant FNT subtable (in particular, you're
// ready to iteratively run command 3 to get data about each contained file/subdir)
int NitroVM_LoadDirectoryDataByIndex(NitroVM* vm, unsigned int dirIndex)
{
    vm->flags |= (1 << NITROVM_FLAG_SYNCHRONOUS);
    vm->regext_abc.a.ptr = vm->linkedHandle;
    vm->regext_abc.c.s32 = 0;
    vm->regext_abc.b.u16.high = 0;
    vm->regext_abc.b.u16.low = dirIndex;
    return NitroVM_ExecuteCommand(vm, NITROVM_OPCODE_GET_DIRECTORY_DATA);
}

int NitroVM_DefaultCommand_Read(NitroVM* vm)
{
    unsigned int oldCursor = vm->fileInfo.cursorPos;
    unsigned int length = vm->regext_abc.c.u32;
    NitroHandle* nitroHandle = vm->linkedHandle;
    void* dst = vm->regext_abc.a.ptr;

    vm->fileInfo.cursorPos = oldCursor + length;
    return nitroHandle->readProc(nitroHandle, dst, oldCursor, length);
}

int NitroVM_DefaultCommand_Write(NitroVM* vm)
{
    unsigned int oldOffset = vm->fileInfo.cursorPos;
    unsigned int length = vm->regext_abc.c.u32;
    NitroHandle* nitroHandle = vm->linkedHandle;
    void* src = vm->regext_abc.a.ptr;

    vm->fileInfo.cursorPos = oldOffset + length;
    return nitroHandle->writeProc(nitroHandle, src, oldOffset, length);
}

struct FNTMainTableEntry
{
    unsigned int subtableOffset; // relative to FNT base
    unsigned short firstContainedFileID;
    unsigned short numDirectoriesOrParentID; // first (root) entry holds num directories, rest hold parent ID
};

int NitroVM_DefaultCommand_GetDirectoryData(NitroVM* vm)
{
    NitroHandle* nitroHandle = vm->linkedHandle;
    FSRegisterTriple* extendedRegs = &vm->regext_abc;
    FNTMainTableEntry tableEntry;
    FSReadDescription readDesc;
    
    readDesc.nitroHandle = nitroHandle;
    readDesc.offset = extendedRegs->b.u16.low * 8 + nitroHandle->nameTableOffsetFast;
    
    int result = Nitro_ReadMetadataBytes(&readDesc, &tableEntry, 8);

    if (result == NITRO_RESULT_SUCCESS)
    {
        vm->dirInfo.accessor = *(NitroDirectoryAccessor*)extendedRegs;
    
        if (extendedRegs->b.u16.high == 0 && extendedRegs->c.u32 == 0)
        {
            vm->dirInfo.accessor.firstFileID = tableEntry.firstContainedFileID;
            vm->dirInfo.accessor.handleSubtableOffset = nitroHandle->nameTableOffsetFast + tableEntry.subtableOffset;
        }
        vm->dirInfo.parentID = tableEntry.numDirectoriesOrParentID & 0xfff;
    }

    return result;
}

struct FileDataStore
{
    NitroHandle* nitroHandle;
    // If holding data for a file, holds the file ID as a 32-bit value
    // (i.e. top 16 bits are zero).
    // If holding data for a directory, the bottom 16 bits hold its ID
    // without the 0xF000 term, and the top 16 bits hold the ID of the first
    // file in the directory.
    FSRegister fileOrDirID;
    // Only used for directory entries
    unsigned int maybeSubtableOffset;
    unsigned int isDirectory; // 1 = directory, 0 = file
    unsigned int stringLength;
    unsigned char name[128];
};

int NitroVM_DefaultCommand_GetFileOrDirectoryNameData(NitroVM* vm)
{
    FileDataStore* pStorage = (FileDataStore*)vm->regext_abc.a.ptr;
    FSReadDescription readDesc;

    readDesc.nitroHandle = vm->linkedHandle;
    readDesc.offset = vm->dirInfo.accessor.handleSubtableOffset;

    unsigned char stringLengthAndType;
    int result = Nitro_ReadMetadataBytes(&readDesc, &stringLengthAndType, 1);

    if (result != 0)
        return result;

    pStorage->stringLength = stringLengthAndType & 0x7f;
    pStorage->isDirectory = ((int)stringLengthAndType >> 7) & 1;
    
    if (pStorage->stringLength == 0)
        return NITRO_RESULT_FAILURE;

    if (!vm->regext_abc.b.u32) // don't skip copying the string
    {
        result = Nitro_ReadMetadataBytes(&readDesc, pStorage->name, pStorage->stringLength);
        if (result != NITRO_RESULT_SUCCESS)
            return result;

        pStorage->name[pStorage->stringLength] = '\0';
    }
    else // do skip over the string
    {
        readDesc.offset += pStorage->stringLength;
    }

    if (pStorage->isDirectory)
    {
        unsigned short directoryID;
        result = Nitro_ReadMetadataBytes(&readDesc, &directoryID, 2);
        if (result != NITRO_RESULT_SUCCESS)
            return result;
        pStorage->nitroHandle = vm->linkedHandle;
        pStorage->fileOrDirID.u16.low = directoryID & 0xfff;
        pStorage->fileOrDirID.u16.high = 0;
        pStorage->maybeSubtableOffset = 0;
    }
    else
    {
        pStorage->nitroHandle = vm->linkedHandle;
        pStorage->fileOrDirID.u32 = vm->dirInfo.accessor.firstFileID;
        vm->dirInfo.accessor.firstFileID++;
    }
    
    vm->dirInfo.accessor.handleSubtableOffset = readDesc.offset;
    return result;
}

int NitroVM_DefaultCommand_GetFileOrDirectoryByName(NitroVM* vm)
{
    FileDataStore storage;
    unsigned char* filePath = (unsigned char*)vm->regext_d.ptr;
    int targetIsDirectory = vm->reg8.s32;

    NitroVM_ExecuteCommand(vm, NITROVM_OPCODE_GET_DIRECTORY_DATA);
    
    // Parse the filename string in terms of tokens (i.e. directory names
    // followed by the final filename).
    if (filePath[0] != '\0')
    {
        do
        {
            int tokenLength = 0;
            // This variable is used for 2 different purposes: mainly to hold
            // a bool (whether the current token refers to a directory or the file)
            // but in the beginning, we use it to hold characters of the string
            int isParsingDirectory;
            goto SkipInitialIncrement;
            while (true)
            {
                tokenLength++;
            SkipInitialIncrement:
                int regularChar = false;
                isParsingDirectory = filePath[tokenLength];
                if (!(isParsingDirectory == '\0' || isParsingDirectory == '/' || isParsingDirectory == '\\'))
                    regularChar = true;
                if (!regularChar)
                    break;
            }

            // If this doesn't run, then isParsingDirectory = '\0' already, i.e. is false
            if (isParsingDirectory != '\0' || targetIsDirectory)
                isParsingDirectory = true;

            if (tokenLength == 0)
                return NITRO_RESULT_FAILURE;
            
            // Special treatment for directory names "." (this dir) and ".." (go up a dir)
            if (*filePath == '.')
            {
                if (tokenLength == 1)
                {
                    filePath++;
                    goto loopEnd;
                }
                // single & instead of &&. Probably originally a typo but means
                // the check doesn't short-circuit (and the bools cast to int)
                if (tokenLength == 2 & filePath[1] == '.')
                {
                    // base_D holds the parent directory index following initial
                    // execution of opcode 2. (If this runs, it'll be updated to the parent of that)
                    if (vm->dirInfo.accessor.dirID != 0)
                        NitroVM_LoadDirectoryDataByIndex(vm, vm->dirInfo.parentID);
                    filePath += 2;
                    goto loopEnd;
                }
            }

            if (tokenLength > 127)
                return NITRO_RESULT_FAILURE;

            vm->regext_abc.a.ptr = &storage;
            vm->regext_abc.b.u32 = 0;
            while (true)
            {
                if (NitroVM_ExecuteCommand(vm, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) != NITRO_RESULT_SUCCESS)
                    return NITRO_RESULT_FAILURE;
                
                if (isParsingDirectory == storage.isDirectory && tokenLength == storage.stringLength
                && CaseInsensitiveStrncmp(filePath, storage.name, tokenLength) == 0)
                    break;
            }

            if (isParsingDirectory)
            {
                vm->regext_abc = *((FSRegisterTriple*)&storage);
                filePath += tokenLength;
                NitroVM_ExecuteCommand(vm, NITROVM_OPCODE_GET_DIRECTORY_DATA);
            }
            else
            {
                if (targetIsDirectory)
                    return NITRO_RESULT_FAILURE;

                volatile FileDataStore& volStorage = storage;
                NitroFileAccessor* output = (NitroFileAccessor*)vm->reg9.ptr;

                NitroHandle* nh = volStorage.nitroHandle;
                unsigned int fileID = volStorage.fileOrDirID.s32;
                output->handle = nh;
                output->fileID = fileID;
                return NITRO_RESULT_SUCCESS;
            }

        loopEnd:
            filePath += filePath[0] != '\0' ? 1 : 0;
        } while (filePath[0] != '\0');
    }

    // I think we only get here if the final token represents a directory,
    // i.e. we're looking for a directory in the first place
    if (!targetIsDirectory)
        return NITRO_RESULT_FAILURE;
    
    // base registers a, b, c follow the right format for NitroDirectoryMetadata
    *((NitroDirectoryAccessor*)vm->reg9.ptr) = vm->dirInfo.accessor;
    return NITRO_RESULT_SUCCESS;
}

int NitroVM_DefaultCommand_GetPath(NitroVM* vm)
{
    FileDataStore storage;
    NitroVM tempVM;

    NitroHandle* nitroHandle = vm->linkedHandle;
    // Initialisation function
    NitroVM_Initialize(&tempVM);
    tempVM.linkedHandle = vm->linkedHandle;
    
    unsigned int targetFileID;

    unsigned int candidateDirID;
    unsigned int numDirectories;
    unsigned int targetDirID;
    
    if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SEARCH_TARGET_IS_DIRECTORY))
    {
        targetDirID = vm->dirInfo.accessor.dirID;
        targetFileID = NITROFS_ID_INVALID;
    }
    else // looking for a file
    {
        targetFileID = vm->fileInfo.fileID;
        if (vm->regext_abc.c.u16.low != 0)
        {
            targetDirID = vm->regext_abc.c.u16.high;
        }
        else
        {
            candidateDirID = 0;
            numDirectories = 0;
            targetDirID = NITROFS_ID_INVALID;
            do
            {
                NitroVM_LoadDirectoryDataByIndex(&tempVM, candidateDirID);
                if (candidateDirID == 0)
                {
                    // in the case of id 0 (the root directory), the 'parent'
                    // actually holds the total number of directories
                    numDirectories = tempVM.dirInfo.parentID;
                }
                // [for command 3] where to store data about the file/directory entry
                tempVM.regext_abc.a.ptr = &storage;
                // [for command 3] don't bother copying out the name
                tempVM.regext_abc.b.u32 = 1;

                if (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS)
                {
                    do {
                        if (!storage.isDirectory && storage.fileOrDirID.u32 == targetFileID)
                        {
                            targetDirID = tempVM.dirInfo.accessor.dirID;
                            break;
                        }
                    } while (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS);
                }

                if (targetDirID != NITROFS_ID_INVALID)
                    break;
            } while (++candidateDirID < numDirectories);
        }
    }
    
    if (targetDirID == NITROFS_ID_INVALID)
    {
        vm->regext_abc.c.u16.low = 0;
        return NITRO_RESULT_FAILURE;
    }

    // Figure out how many bytes we'll need to write.
    // Looks like the assumption is that if not zero, it's because this
    // instruction has previously run with the same file & so the stored
    // values are already correct & don't need to be recomputed
    if (vm->regext_abc.c.u16.low == 0)
    {
        unsigned int ancestorDirID;
        int totalWriteSize = 0;
        if (nitroHandle->signature <= 0xff)
            totalWriteSize += 1;
        else if (nitroHandle->signature <= 0xff00)
            totalWriteSize += 2;
        else
            totalWriteSize += 3;

        totalWriteSize += 2;
        // If dealing with a file, sum the length of its name now
        if (targetFileID != NITROFS_ID_INVALID)
            totalWriteSize += storage.stringLength;

        ancestorDirID = targetDirID;
        // Loop through ancestor folders (unless we're in the root)
        if (targetDirID != 0)
        {
            NitroVM_LoadDirectoryDataByIndex(&tempVM, targetDirID);
            do 
            {
                // Get data about the previous directory's parent
                NitroVM_LoadDirectoryDataByIndex(&tempVM, tempVM.dirInfo.parentID);
                tempVM.regext_abc.a.ptr = &storage;
                tempVM.regext_abc.b.u32 = 1; // Don't bother copying the string

                if (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS)
                {
                    do
                    {
                        if (!storage.isDirectory)
                            continue;
                        
                        if (storage.fileOrDirID.u16.low != ancestorDirID)
                            continue;
    
                        totalWriteSize += storage.stringLength + 1;
                        break;
                    } while (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS);
                }
                // This still holds the parent id parameter passed to
                // LoadDirectoryDataByIndex
                ancestorDirID = tempVM.dirInfo.accessor.dirID;
            } while (ancestorDirID != 0);
        }
        // +1 to account for null terminator
        vm->regext_abc.c.u16.low = totalWriteSize + 1;
        vm->regext_abc.c.u16.high = targetDirID;
    }
    
    if (!vm->regext_abc.a.ptr)
        return NITRO_RESULT_SUCCESS;

    unsigned int backWriteLocation = vm->regext_abc.c.u16.low;
    unsigned char* writeDst = (unsigned char*)vm->regext_abc.a.ptr;
    if (vm->regext_abc.b.u32 < backWriteLocation)
        return NITRO_RESULT_FAILURE;

    {
        // Ignore the weird assembly, there was a weird quirk of the code that
        // the compiler never produces unless you do something stupid like this.
        // tl;dr signature holds the value nitroHandle->signature, and
        // uselessZero is just 0 and used for a pointless operation later
        unsigned int uselessZero;
        unsigned int signature = (unsigned int)nitroHandle;
        __asm("mov uselessZero, 0");
        signature = *(unsigned int*)signature;
        DECLARE_ASM_NOP();

        int signatureLength;
        if (signature <= 0xff)
            signatureLength = 1;
        else if (signature <= 0xff00) // shouldn't this really be 0xffff?
            signatureLength = 2;
        else
            signatureLength = 3;
    
        VectorizedInvertedMemcpy(&nitroHandle->signature, writeDst, signatureLength);
        signatureLength = uselessZero + signatureLength;
        // copy ":/"
        VectorizedInvertedMemcpy(data_020f2288, writeDst + signatureLength, 2);
    }

    NitroVM_LoadDirectoryDataByIndex(&tempVM, targetDirID);
    if (targetFileID != NITROFS_ID_INVALID)
    {
        tempVM.regext_abc.a.ptr = &storage;
        tempVM.regext_abc.b.u32 = 0; // This time, do copy the string
        if (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS)
        {
            do 
            {
                if (!storage.isDirectory && storage.fileOrDirID.u32 == targetFileID)
                    break;
            } while (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS);
        }
        // The copied string is null-terminated, and we want to also copy the null terminator
        int copySize = storage.stringLength + 1;
        VectorizedInvertedMemcpy(storage.name, writeDst + backWriteLocation - copySize, copySize);
        backWriteLocation -= copySize;
    }
    else
    {
        *(writeDst + backWriteLocation - 1) = '\0';
        backWriteLocation--;
    }

    if (targetDirID != 0)
    {
        do
        {
            NitroVM_LoadDirectoryDataByIndex(&tempVM, tempVM.dirInfo.parentID);
            tempVM.regext_abc.a.ptr = &storage;
            tempVM.regext_abc.b.u32 = 0;
            *(writeDst + backWriteLocation - 1) = '/';
            backWriteLocation--;

            if (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS)
            {
                do
                {
                    if (!storage.isDirectory) continue;
                    if (storage.fileOrDirID.u16.low != targetDirID) continue;
                    
                    unsigned int tokenLength = storage.stringLength;
                    VectorizedInvertedMemcpy(storage.name,
                        writeDst + backWriteLocation - tokenLength, tokenLength);
                    backWriteLocation -= tokenLength;
                    break;
                } while (NitroVM_ExecuteCommand(&tempVM, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA) == NITRO_RESULT_SUCCESS);
            }
            targetDirID = tempVM.dirInfo.accessor.dirID;
        } while (targetDirID != 0);
    }

    return NITRO_RESULT_SUCCESS;
}

int NitroVM_DefaultCommand_GetFATEntry(NitroVM* vm)
{
    unsigned int offsets[2]; // [0] = start, [1] = end
    FSReadDescription readHandle;

    unsigned int entryIdx = vm->regext_abc.b.u32;
    if (vm->linkedHandle->fatSize <= entryIdx * 8)
        return 1;
    
    readHandle.nitroHandle = vm->linkedHandle;

    // Dumb way to write readHandle.offset = fatOffset + entryIdx * 8
    int off = readHandle.nitroHandle->fatOffsetFast;
    readHandle.offset = entryIdx * 8;
    off += readHandle.offset;
    readHandle.offset = off;

    int result = Nitro_ReadMetadataBytes(&readHandle, &offsets, 8);
    if (result != NITRO_RESULT_SUCCESS)
        return result;

    vm->regext_abc.a.u32 = offsets[0];
    vm->regext_abc.b.u32 = offsets[1];
    vm->regext_abc.c.u32 = entryIdx;

    return NitroVM_ExecuteCommand(vm, NITROVM_OPCODE_COPY_REGISTERS);
}

int NitroVM_DefaultCommand_CopyRegisters(NitroVM* vm)
{
    vm->fileInfo.startOffset = vm->regext_abc.a.u32;
    vm->fileInfo.cursorPos = vm->regext_abc.a.u32;
    vm->fileInfo.endOffset = vm->regext_abc.b.u32;
    vm->fileInfo.fileID = vm->regext_abc.c.u32;
    return NITRO_RESULT_SUCCESS;
}

int NitroVM_DefaultCommand_Nop(NitroVM* vm)
{
    return NITRO_RESULT_SUCCESS;
}

unsigned int Nitro_CalculateSignature(const char* str, int len)
{
    unsigned int signature = 0;

    if (len <= 3)
    {
        int idx = 0; 
        if (len > 0)
        {   
            int bitshift = 0;
            
            do {
                if ((unsigned char)str[idx] == '\0')
                    break;
                
                // Convert to lowercase
                unsigned int charValue = (unsigned char)str[idx] - 'A';
                if (charValue <= 'Z' - 'A')
                    charValue += 'a';
                else
                    charValue += 'A';

                idx++;
                signature |= charValue << bitshift;
                bitshift += 8;
            } while (idx < len);
        }
    }

    return signature;
}

int DefaultNitroReadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len)
{
    VectorizedInvertedMemcpy((const unsigned char*)handle->pFileImage + offset, dst, len);
    return NITRO_RESULT_SUCCESS;
}

int DefaultNitroWriteProc(NitroHandle* handle, const void* src, unsigned int offset, unsigned int len)
{
    VectorizedInvertedMemcpy(src, (unsigned char*)handle->pFileImage + offset, len);
    return NITRO_RESULT_SUCCESS;
}

int MemoryMappedMetadataReadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len)
{
    VectorizedInvertedMemcpy((void*)offset, dst, len);
    return NITRO_RESULT_SUCCESS;
}

NitroVM* NitroHandle_AdvanceCommandQueue(NitroHandle* handle)
{
    int oldState = DisableIRQInterrupts();
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_VM_LIST_DIRTY))
    {
        handle->flags &= ~(1 << NITROHANDLE_FLAG_VM_LIST_DIRTY);
        NitroVM* currentVM = handle->linkToFirstVM.pNext;
        if (currentVM != NULL)
        {
            do {
                NitroVM* nextVM = currentVM->links.pNext;
                if (GET_FLAG_BIT(currentVM->flags, NITROVM_FLAG_MARKED_FOR_UNLINK_FROM_HANDLE))
                {
                    if (handle->linkToFirstVM.pNext == currentVM)
                        handle->linkToFirstVM.pNext = nextVM;
                    NitroVM_UnlinkAndStoreResult(currentVM, NITRO_RESULT_INVALID_HANDLE);
                    // Effect of this: if last VM in the list marked for unlink,
                    // need to do another pass through...?
                    if (nextVM == NULL)
                        nextVM = handle->linkToFirstVM.pNext;
                }
                currentVM = nextVM;
            } while (currentVM != NULL);
        }
    }

    if (!GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_AWAITING_BUS_RELEASE) && !GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_QUEUE_PAUSED))
    {
        NitroVM* vm = handle->linkToFirstVM.pNext;
        if (vm != NULL)
        {
            int flagCleared = !GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_NDS_BUS_HELD);
            if (flagCleared)
                handle->flags |= (1 << NITROHANDLE_FLAG_NDS_BUS_HELD);
            SetIRQInterruptState(oldState);
            if (flagCleared && (handle->overrideOpcodeFlags & (1 << NITROVM_OPCODE_ACQUIRE_NDS_BUS)))
                handle->instructionOverride(vm, NITROVM_OPCODE_ACQUIRE_NDS_BUS);
            oldState = DisableIRQInterrupts();
            vm->flags |= (1 << NITROVM_FLAG_READY_TO_EXECUTE);
            if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SYNCHRONOUS))
            {
                UnblockContexts(&vm->blockedContexts);
                SetIRQInterruptState(oldState);
                return NULL;
            }
            else
            {
                SetIRQInterruptState(oldState);
                return vm;
            }
        }
    }

    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_NDS_BUS_HELD))
    {
        handle->flags &= ~(1 << NITROHANDLE_FLAG_NDS_BUS_HELD);
        if (handle->overrideOpcodeFlags & (1 << NITROVM_OPCODE_RELEASE_NDS_BUS))
        {
            NitroVM tempVM;
            NitroVM_Initialize(&tempVM);
            tempVM.linkedHandle = handle;
            handle->instructionOverride(&tempVM, NITROVM_OPCODE_RELEASE_NDS_BUS);
        }
    }

    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_AWAITING_BUS_RELEASE))
    {
        handle->flags = (handle->flags & ~(1 << NITROHANDLE_FLAG_AWAITING_BUS_RELEASE)) | (1 << NITROHANDLE_FLAG_QUEUE_PAUSED);
        UnblockContexts(&handle->busReleaseBlock);
    }

    SetIRQInterruptState(oldState);
    return NULL;
}

void NitroVM_ProcessReadyCommandQueueEntries(NitroVM* queueHead)
{
    NitroVM* vm = queueHead;
    NitroHandle* handle = vm->linkedHandle;
    if (vm != NULL)
    {
        do
        {
            int oldState = DisableIRQInterrupts();
            vm->flags |= (1 << NITROVM_FLAG_READY_TO_EXECUTE);
            if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SYNCHRONOUS))
            {
                UnblockContexts(&vm->blockedContexts);
                SetIRQInterruptState(oldState);
                break;
            }
            vm->flags |= (1 << NITROVM_FLAG_EXECUTING_FROM_QUEUE);
            SetIRQInterruptState(oldState);

            if (NitroVM_ExecuteCommand(vm, vm->pendingCommand) == NITRO_RESULT_TASK_STILL_RUNNING)
                break;

            vm = NitroHandle_AdvanceCommandQueue(handle);
        } while (vm != NULL);
    }
}

CBool NitroVM_ExecuteAndUnlink(NitroVM* vm)
{
    int result = NitroVM_ExecuteCommand(vm, vm->pendingCommand);
    NitroVM_UnlinkAndStoreResult(vm, result);

    NitroVM* maybeMainVM = NitroHandle_AdvanceCommandQueue(vm->linkedHandle);
    if (maybeMainVM != NULL)
        NitroVM_ProcessReadyCommandQueueEntries(maybeMainVM);

    return vm->storedResult == NITRO_RESULT_SUCCESS;
}

CBool NitroVM_QueueCommand(NitroVM* vm, int opcode)
{
    NitroHandle* handle = vm->linkedHandle;
    vm->pendingCommand = opcode;
    vm->storedResult = NITRO_RESULT_UNDEFINED;
    int opcodeMask = 1 << opcode;
    vm->flags |= (1 << NITROVM_FLAG_IN_HANDLE_QUEUE);

    int oldState = DisableIRQInterrupts();
    if (handle->flags & (1 << NITROHANDLE_FLAG_DESTRUCTION_UNDERWAY))
    {
        NitroVM_UnlinkAndStoreResult(vm, NITRO_RESULT_INVALID_HANDLE);
        SetIRQInterruptState(oldState);
        return false;
    }

    // if opcode is between 2 and 8 (inclusive)
    if (opcodeMask & 0x1fc)
        vm->flags |= (1 << NITROVM_FLAG_SYNCHRONOUS);

    NitroVM* lastAttachedVM;
    // At least one of these needs to be a volatile read
    NitroVM* previous = *(NitroVM* volatile*)&vm->links.pPrev;
    NitroVM* next = *(NitroVM* volatile*)&vm->links.pNext;
    
    // Move vm to end of the list
    if (previous)
        previous->links.pNext = next;
    
    lastAttachedVM = (NitroVM*)&handle->linkToFirstVM;
    if (next)
        next->links.pPrev = previous;
    
    if (lastAttachedVM->links.pNext)
    {
        NitroVM* loopVM = lastAttachedVM->links.pNext;
        do {
            lastAttachedVM = loopVM;
            loopVM = loopVM->links.pNext;
        } while (loopVM != NULL);
    }
    
    lastAttachedVM->links.pNext = vm;
    vm->links.pPrev = lastAttachedVM;
    vm->links.pNext = NULL;

    if (!GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_QUEUE_PAUSED) && !GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_NDS_BUS_HELD))
    {
        // Claim: if we get here, the queue was empty, so can execute this VM already
        handle->flags |= (1 << NITROHANDLE_FLAG_NDS_BUS_HELD);
        SetIRQInterruptState(oldState);
        if (handle->overrideOpcodeFlags & (1 << NITROVM_OPCODE_ACQUIRE_NDS_BUS))
            handle->instructionOverride(vm, NITROVM_OPCODE_ACQUIRE_NDS_BUS);
        int oldState = DisableIRQInterrupts();
        vm->flags |= (1 << NITROVM_FLAG_READY_TO_EXECUTE);
        if (!GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SYNCHRONOUS))
        {
            SetIRQInterruptState(oldState);
            NitroVM_ProcessReadyCommandQueueEntries(vm);
            return true;
        }
        SetIRQInterruptState(oldState);
    }
    else
    {
        if (!GET_FLAG_BIT(vm->flags, NITROVM_FLAG_SYNCHRONOUS))
        {
            SetIRQInterruptState(oldState);
            return true;
        }

        do
        {
            BlockCurrentContext(&vm->blockedContexts);
        } while (!(vm->flags & (1 << NITROVM_FLAG_READY_TO_EXECUTE)));
        SetIRQInterruptState(oldState);
    }
    return NitroVM_ExecuteAndUnlink(vm);
}