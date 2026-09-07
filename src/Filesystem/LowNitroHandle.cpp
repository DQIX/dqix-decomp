#include "Filesystem/FSInnerDefs.h"
#include "System/Memory.h"
#include "System/Interrupts.h"
#include <globaldefs.h>

#pragma optimize_for_size off

int DefaultNitroReadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len);
int DefaultNitroWriteProc(NitroHandle* handle, const void* src, unsigned int offset, unsigned int len);
int MemoryMappedMetadataReadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len);

void NitroHandle_Initialize(NitroHandle* handle)
{
    VectorizedMemset(handle, 0, sizeof(NitroHandle));
    handle->taskWaitBlock.last = NULL;
    handle->taskWaitBlock.first = NULL;
    handle->busReleaseBlock.last = NULL;
    handle->busReleaseBlock.first = NULL;
}

NitroHandle* NitroHandle_FindBySignature(const char* str, int len)
{
    unsigned int targetSig = Nitro_CalculateSignature(str, len);
    int oldState = DisableIRQInterrupts();

    NitroHandle* handle = data_02111728.firstHandle;

    while (handle != NULL && handle->signature != targetSig)
    {
        handle = handle->pNextHandle;
    }

    SetIRQInterruptState(oldState);
    return handle;
}

CBool NitroHandle_AddToHandleList(NitroHandle* handle, const char* str, int len)
{
    CBool result = false;
    int oldState = DisableIRQInterrupts();
    if (NitroHandle_FindBySignature(str, len) == NULL)
    {
        if (data_02111728.firstHandle == NULL)
        {
            data_02111728.firstHandle = handle;
            data_02111728.romFSRoot.handle = handle;
            data_02111728.romFSRoot.handleSubtableOffset = 0;
            data_02111728.romFSRoot.firstFileID = 0;
            data_02111728.romFSRoot.dirID = 0;
        }
        else
        {
            // At the end of this, currentEntry holds the last entry in the list
            // and nextEntry is null (points past the end)
            NitroHandle* currentEntry = data_02111728.firstHandle;
            NitroHandle* nextEntry = currentEntry->pNextHandle;
            if (nextEntry != NULL)
            {
                do
                {
                    currentEntry = nextEntry;
                    nextEntry = nextEntry->pNextHandle;
                } while (nextEntry != NULL);
            }

            currentEntry->pNextHandle = handle;
            handle->pPrevHandle = currentEntry;
        }
        handle->signature = Nitro_CalculateSignature(str, len);
        result = true;
        handle->flags |= (1 << NITROHANDLE_FLAG_IS_IN_MAIN_LIST);
    }

    SetIRQInterruptState(oldState);
    return result;
}

void NitroHandle_RemoveFromHandleList(NitroHandle* handle)
{
    if (handle->signature == 0)
        return;

    int oldState = DisableIRQInterrupts();

    if (handle->pNextHandle != NULL)
        handle->pNextHandle->pPrevHandle = handle->pPrevHandle;

    if (handle->pPrevHandle != NULL)
        handle->pPrevHandle->pNextHandle = handle->pNextHandle;

    handle->signature = 0;
    handle->pPrevHandle = NULL;
    handle->pNextHandle = NULL;
    handle->flags &= ~(1 << NITROHANDLE_FLAG_IS_IN_MAIN_LIST);

    if (data_02111728.romFSRoot.handle == handle)
    {
        data_02111728.romFSRoot.handle = data_02111728.firstHandle;
        data_02111728.romFSRoot.handleSubtableOffset = 0;
        data_02111728.romFSRoot.firstFileID = 0;
        data_02111728.romFSRoot.dirID = 0;
    }

    SetIRQInterruptState(oldState);
}

CBool NitroHandle_Populate(NitroHandle* handle, void* image,
        unsigned int fatOffset, unsigned int fatSize,
        unsigned int fntOffset, unsigned int fntSize,
        NitroHandle::ReadProc loadProc, NitroHandle::WriteProc saveProc)
{
    handle->pFileImage = image;
    handle->fatSize = fatSize;
    handle->fatOffsetFast = handle->fatOffset = fatOffset;
    handle->nameTableSize = fntSize;
    handle->nameTableOffsetFast = handle->nameTableOffset = fntOffset;

    handle->readProc = (loadProc == NULL) ? &DefaultNitroReadProc : loadProc;
    handle->writeProc = (saveProc == NULL) ? &DefaultNitroWriteProc : saveProc;

    handle->fastReadProc = handle->readProc;
    handle->tableRawPointer = NULL;
    handle->flags |= (1 << NITROHANDLE_FLAG_IS_POPULATED);
    return true;
}

CBool NitroHandle_Destroy(NitroHandle* handle)
{
    int oldState = DisableIRQInterrupts();
    
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_IS_POPULATED) != 0)
    {
        // Silly unused volatile read of the flags. Assembly is extra misleading,
        // can make it look like it's the 2nd argument, but this is wrong
        CBool paused = (handle->flags, NitroHandle_Pause(handle));
        handle->flags |= (1 << NITROHANDLE_FLAG_DESTRUCTION_UNDERWAY);
        
        NitroVM* machine = handle->linkToFirstVM.pNext;
        if (machine != NULL)
        {
            do {
                NitroVM* next = machine->links.pNext;
                NitroVM_UnlinkAndStoreResult(machine, NITRO_RESULT_INVALID_HANDLE);
                machine = next;
            } while (machine != NULL);
        }
        handle->linkToFirstVM.pNext = NULL;
        if (paused)
            NitroHandle_Unpause(handle);

        handle->pFileImage = NULL;
        handle->fatOffsetFast = 0;
        handle->fatSize = 0;
        handle->nameTableOffsetFast = 0;
        handle->nameTableSize = 0;
        handle->nameTableOffset = 0;
        handle->fatOffset = 0;

        handle->flags &= ~((1 << NITROHANDLE_FLAG_DESTRUCTION_UNDERWAY) | (1 << NITROHANDLE_FLAG_VM_LIST_DIRTY) | (1 << NITROHANDLE_FLAG_IS_POPULATED));
    }
    SetIRQInterruptState(oldState);
    return true;
}

unsigned int NitroHandle_LoadFileTables(NitroHandle* handle, void* into, unsigned int capacity)
{
    unsigned int neededSpace = (handle->fatSize + handle->nameTableSize + 0x3f) & ~0x1f;

    if (neededSpace <= capacity)
    {
        void* alignedPtr = (void*)(((unsigned int)into + 0x1f) & ~0x1f);
        NitroVM vm;
        NitroVM_Initialize(&vm);
        if (NitroVM_PrepareRead(&vm, handle, 
            handle->fatOffsetFast,
            handle->fatOffsetFast + handle->fatSize, -1))
        {
            if (NitroVM_ReadSync(&vm, alignedPtr, handle->fatSize) < 0)
                VectorizedMemset(alignedPtr, 0, handle->fatSize);
            NitroVM_FinishRead(&vm);
        }
        handle->fatOffsetFast = (unsigned int)alignedPtr;
        alignedPtr = (void*)((unsigned int)alignedPtr + handle->fatSize);

        if (NitroVM_PrepareRead(&vm, handle, 
            handle->nameTableOffsetFast,
            handle->nameTableOffsetFast + handle->nameTableSize, -1))
        {
            if (NitroVM_ReadSync(&vm, alignedPtr, handle->nameTableSize) < 0)
                VectorizedMemset(alignedPtr, 0, handle->nameTableSize);
            NitroVM_FinishRead(&vm);
        }

        handle->nameTableOffsetFast = (unsigned int)alignedPtr;
        handle->tableRawPointer = into;
        handle->fastReadProc = &MemoryMappedMetadataReadProc;
        handle->flags |= (1 << NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY);
    }

    return neededSpace;
}

void* NitroHandle_ReleaseFileTables(NitroHandle* handle)
{
    void* oldPtr = NULL;
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_IS_POPULATED))
    {
        int paused = NitroHandle_Pause(handle);
        if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY))
        {
            handle->flags &= ~(1 << NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY);
            oldPtr = handle->tableRawPointer;
            handle->tableRawPointer = NULL;
            handle->fatOffsetFast = handle->fatOffset;
            handle->nameTableOffsetFast = handle->nameTableOffset;
            handle->fastReadProc = handle->readProc;
        }
        if (paused)
            NitroHandle_Unpause(handle);
    }
    return oldPtr;
}

CBool NitroHandle_Pause(NitroHandle* handle)
{
    int oldState = DisableIRQInterrupts();
    CBool changeState = GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_QUEUE_PAUSED) == 0;
    if (changeState)
    {
        int busHeld = GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_NDS_BUS_HELD);
        int oldFlags = handle->flags;
        if (busHeld)
        {
            handle->flags = oldFlags | (1 << NITROHANDLE_FLAG_AWAITING_BUS_RELEASE);
            do {
                BlockCurrentContext(&handle->busReleaseBlock);
            } while (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_AWAITING_BUS_RELEASE));
        }
        else
        {
            handle->flags = oldFlags | (1 << NITROHANDLE_FLAG_QUEUE_PAUSED);
        }
    }

    SetIRQInterruptState(oldState);
    return changeState;
}

CBool NitroHandle_Unpause(NitroHandle* handle)
{
    NitroVM* machine = NULL;
    int oldState = DisableIRQInterrupts();

    CBool skip = GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_QUEUE_PAUSED) == 0;
    if (!skip)
    {
        handle->flags &= ~(1 << NITROHANDLE_FLAG_QUEUE_PAUSED);
        machine = NitroHandle_AdvanceCommandQueue(handle);
    }

    SetIRQInterruptState(oldState);
    if (machine != NULL)
        NitroVM_ProcessReadyCommandQueueEntries(machine);
    return skip;
}

void NitroHandle_SetOpcodeOverride(NitroHandle* handle, NitroHandle::CommandOverride fnOverride, unsigned int mask)
{
    if (mask == 0)
        fnOverride = NULL;
    else if (fnOverride == NULL)
        mask = 0;
    
    handle->instructionOverride = fnOverride;
    handle->overrideOpcodeFlags = mask;
}

void NitroHandle_OnTaskCompletion(NitroHandle* handle, int result)
{
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_ASYNC_COMMAND_IN_PROGRESS))
    {
        int previousFlags = handle->flags;
        NitroVM* firstVM = handle->linkToFirstVM.pNext;
        handle->flags = previousFlags & ~(1 << NITROHANDLE_FLAG_ASYNC_COMMAND_IN_PROGRESS);
        NitroVM_UnlinkAndStoreResult(firstVM, result);
        NitroVM* fs = NitroHandle_AdvanceCommandQueue(handle);
        if (fs != NULL)
            NitroVM_ProcessReadyCommandQueueEntries(fs);
    }
    else
    {
        NitroVM* firstVM = handle->linkToFirstVM.pNext;
        int oldState = DisableIRQInterrupts();
        firstVM->storedResult = result;
        handle->flags &= ~(1 << NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS);
        UnblockContexts(&handle->taskWaitBlock);
        SetIRQInterruptState(oldState);
    }
}

void InitializeROMFilesystem(int dmaChannel)
{
    if (data_02111738)
        return;
    data_02111738 = true;
    InitializeROMFilesystem_Internal(dmaChannel);
}

void NitroVM_Initialize(NitroVM* vm)
{
    vm->links.pPrev = NULL;
    vm->links.pNext = NULL;
    vm->blockedContexts.last = NULL;
    vm->blockedContexts.first = NULL;
    vm->linkedHandle = NULL;
    vm->pendingCommand = NITROVM_OPCODE_MAYBE_INVALID;
    vm->flags = 0;
}

int NitroVM_SearchFileOrDirectory(NitroVM* vm, const char* inPath,
        NitroFileAccessor* outFileData, NitroDirectoryAccessor* outDirData)
{
    NitroDirectoryAccessor accessor;
    // Cast to unsigned and remove const for convenience
    unsigned char* path = (unsigned char*)inPath;
    if (path[0] == '/' || path[0] == '\\')
    {
        accessor.handle = data_0211172c.handle;
        accessor.dirID = 0;
        accessor.handleSubtableOffset = 0;
        accessor.firstFileID = 0;
        path++;
    }
    else
    {
        accessor = data_0211172c;
        int signatureIdx = 0;
        do
        {
            if (path[signatureIdx] == '\0' || path[signatureIdx] == '/' || path[signatureIdx] == '\\')
                break;
            if (path[signatureIdx] != ':')
                continue;
            
            NitroHandle* handle = NitroHandle_FindBySignature(inPath, signatureIdx);

            if (handle == NULL)
                return false;
            if (!GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_IS_POPULATED))
                return false;

            accessor.handle = handle;
            accessor.handleSubtableOffset = 0;
            accessor.firstFileID = 0;
            accessor.dirID = 0;
            path += signatureIdx + 1;
            if (path[0] == '/' || path[0] == '\\')
                path++;
            break;
        } while (++signatureIdx <= 3);
    }

    vm->linkedHandle = accessor.handle;
    vm->regext_d.ptr = (char*)path;
    vm->regext_abc = *(FSRegisterTriple*)&accessor;
    if (outDirData != NULL)
    {
        vm->args_GetFileOrDirectoryByName.searchForDirectory = 1;
        vm->args_GetFileOrDirectoryByName.output = outDirData;
    }
    else
    {
        vm->args_GetFileOrDirectoryByName.searchForDirectory = 0;
        vm->args_GetFileOrDirectoryByName.output = outFileData;
    }

    return NitroVM_QueueCommand(vm, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_BY_NAME);
}

int NitroVM_Read(NitroVM* vm, void* dst, int capacity, CBool async)
{
    int srcStart = vm->fileInfo.cursorPos;
    int srcEnd = vm->fileInfo.endOffset;
    
    int remainingLength = srcEnd - srcStart;
    
    int lengthToCopy = capacity;
    unsigned int copyOfCapacity = capacity; // unsigned fixes register stuff
    
    if (lengthToCopy > remainingLength)
        lengthToCopy = remainingLength;
    
    if (lengthToCopy < 0)
        lengthToCopy = 0;
    
    vm->regext_abc.a.ptr = (char*)dst;
    vm->regext_abc.b.u32 = copyOfCapacity;
    vm->regext_abc.c.u32 = lengthToCopy;
    
    if (!async)
        vm->flags |= (1 << NITROVM_FLAG_SYNCHRONOUS);
    
    NitroVM_QueueCommand(vm, NITROVM_OPCODE_LOAD);
    
    if (!async)
    {
        if (NitroVM_AwaitCommandCompletion(vm))
            lengthToCopy = vm->fileInfo.cursorPos - srcStart;
        else
            lengthToCopy = -1;
    }

    return lengthToCopy;
}