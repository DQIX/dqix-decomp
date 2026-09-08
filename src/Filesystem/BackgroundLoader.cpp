#include "Filesystem/BackgroundLoader.h"
#include "std_library_functions.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/FileIO.h"
#include "Resource/ResourceMutex.h"

#pragma dont_inline on

#ifdef jpn
#define func_020c9be0 func_020cb6ac

#define func_020d97a8 func_020db1b4
#define func_020d9828 func_020db234

#define data_020ef8a4 data_020ef794
#define data_020ef908 data_020ef7f8
#define data_020ef90f data_020ef7ff
#endif

struct Struct_02104304
{
    unsigned short taskCounter_;
    BackgroundLoader* globalInstace_;
    char contextStackSpace_[0x800];
} static s_loaderInstanceData;

extern "C"
{
    // get system language
    int func_0200fb08(BattleStruct*);

    // abort() or similar
    void func_020c9be0();

    // Populates context with priority = relPrio + 16
    void func_020d97a8(ProcessorContext* context, void* stackSpace, unsigned stackSize, int relPrio, const void* entry, int arg);
    void func_020d9828(ProcessorContext* context); // thunk for MarkContextReadyAndSwitch

    char* data_020ef8a4[]; // array of subdirectories within data/

    char data_020ef908[]; // "%s%s%s"
    char data_020ef90f[]; // "data/"
}

void BackgroundLoader::Task::ZeroInitialize()
{
    outerFilename_[0] = '\0';
    innerFilename_[0] = '\0';
    taskID_ = 0;
    status_ = BackgroundLoader::TaskStatus_Invalid;
    externalAllocator_ = NULL;
    pFileData = NULL;
    fileLengthOrOverlayID_ = 0;
    type_ = 0;
    scratchAlloc_.firstWord_ = 0;
    scratchAlloc_.numWords_ = 0;
}

void BackgroundLoaderThreadFunction(BackgroundLoader *loader)
{
    while (true)
    {
        loader->Process();
    }
}

bool BackgroundLoader::Task::GetFullFilename(char *outBuffer)
{
    sprintf(outBuffer, data_020ef908, // "%s%s%s"
        data_020ef90f, // "data/"
        data_020ef8a4[containerDirectoryIndex_], // e.g. "ani/" or "bin/tbox/"
        outerFilename_);
    return true;
}

BackgroundLoader* BackgroundLoader::GetInstance()
{
    return s_loaderInstanceData.globalInstace_;
}

void BackgroundLoader::FreeAllocationsGlobal()
{
    if (s_loaderInstanceData.globalInstace_)
        s_loaderInstanceData.globalInstace_->MaybeFreeAllocations();
}

void BackgroundLoader::AddLockGlobal()
{
    if (s_loaderInstanceData.globalInstace_)
        s_loaderInstanceData.globalInstace_->AddLock();
}

void BackgroundLoader::RemoveLockGlobal()
{
    if (s_loaderInstanceData.globalInstace_)
        s_loaderInstanceData.globalInstace_->RemoveLock();
}

void BackgroundLoader::InitializeOrReset()
{
    reader_.ZeroInitialize();
    s_loaderInstanceData.globalInstace_ = this;

    scratchSpace_ = 0;
    scratchSpaceSize_ = 0;
    rightmostAllocationByte_ = 0;
    scratchRightArenaUsage_ = 0;
    unknown_120_ = 0x10000;
    numPendingTasks_ = 0;
    processLockBits_ = 0; 

    flags_78c_0_ = true;
    flags_78c_3_ = false;
    flagMaybeGP2OperationInFlight_ = false;

    BackgroundLoader::Task* pTask = &queuedTasks_[0];
    for (int i = 0; i < 24; i++, pTask++)
    {
        pTask->ZeroInitialize();
    }
}

void BackgroundLoader::Populate(void* fileLoadSpace, unsigned int spaceCapacity, int relativePrio)
{
    InitializeOrReset();
    scratchSpace_ = fileLoadSpace;
    scratchSpaceSize_ = spaceCapacity;
    processLockBits_ = 0;
    flags_78c_1_ = false;
    flags_78c_0_ = true;
    flags_78c_2_ = true;

    func_020d97a8(&context_, s_loaderInstanceData.contextStackSpace_,
        sizeof(s_loaderInstanceData.contextStackSpace_), relativePrio,
        &BackgroundLoaderThreadFunction, (int)this);
    func_020d9828(&context_);
}

void BackgroundLoader::MaybeWaitIdle()
{
    MaybeFreeAllocations();
    AddLock();
    while (true)
    {
        LockResourceMutex();
        bool keepWaiting = true;
        if (!flags_78c_3_ && flagMaybeGP2OperationInFlight_ == false)
            keepWaiting = false;

        UnlockResourceMutex();
        if (!keepWaiting)
            return;
        SleepIfResourceMutexNotLocked(1);
    }
}

void BackgroundLoader::AddLock()
{
    if (numPendingTasks_ == 0 && processLockBits_ == 0)
        return;

    processLockBits_ <<= 1;
    processLockBits_ |= 1;
}

void BackgroundLoader::RemoveLock()
{
    if (processLockBits_ == 0)
        return;

    processLockBits_ >>= 1;
    if (processLockBits_ == 0)
    {
        flags_78c_2_ = false;
        if (numPendingTasks_ != 0)
            SleepIfResourceMutexNotLocked(1);
    }
}

void BackgroundLoader::RemoveAllLocks()
{
    if (processLockBits_ != 0)
        processLockBits_ = 0;

    flags_78c_2_ = false;

    if (numPendingTasks_ != 0)
        SleepIfResourceMutexNotLocked(1);
}

int BackgroundLoader::QueueFileTask(const char* filename, int type, const char* innerFile, SafeAllocator* alloc)
{
    int newID = -1;
    LockResourceMutex();
#if defined(usa)
    int language = func_0200fb08(GetBattleStruct());
    char replacedFilename[80] = { 0 };
    StringReplaceLanguageTag(filename, replacedFilename, language);
#endif

    if (filename != NULL)
    {
        if (numPendingTasks_ >= 24)
            func_020c9be0();
        else
        {   
            BackgroundLoader::Task* entry = &queuedTasks_[numPendingTasks_];
            entry->taskID_ = s_loaderInstanceData.taskCounter_;
            entry->status_ = BackgroundLoader::TaskStatus_Unallocated;
            entry->externalAllocator_ = alloc;
            entry->pFileData = NULL;
            entry->type_ = type;
            if (type == BackgroundLoader::TaskType_LoadFromGP2 && innerFile != NULL)
            {
#if defined(usa)
                char replacedInnerFile[128] = { 0 };
                StringReplaceLanguageTag(innerFile, replacedInnerFile, language);
                strcpy(entry->innerFilename_, replacedInnerFile);
#elif defined(jpn)     
                strcpy(entry->innerFilename_, innerFile);     
#endif
            }
#if defined(jpn)
            char replacedFilename[80];
            strcpy(replacedFilename, filename);
#endif
            for (char* ptr = replacedFilename; *ptr != '\0'; ptr++)
            {
                if (*ptr == '\\')
                    *ptr = '/';
            }

            bool success;
            // presumably this is used because the filename might be like
            // "ROM:/data/bin/blah.gp2" and you want to strip the "ROM:/" prefix
            const char* pathTrueStart = strstr(replacedFilename, data_020ef90f);
            if (pathTrueStart == NULL)
                success = false;
            else
            {
                int subdirIdx;
                const char* candidateSubdir;
                const char* innerDirPtr = pathTrueStart + strlen(data_020ef90f);
                for (subdirIdx = 0; (candidateSubdir = data_020ef8a4[subdirIdx]) != NULL; subdirIdx++)
                {
                    if (strstr(innerDirPtr, candidateSubdir) == innerDirPtr)
                    {
                        unsigned int subdirLength = strlen(candidateSubdir);
                        unsigned int filenameLength = strlen(innerDirPtr + subdirLength);
                        if (filenameLength + 1 > 0x18)
                            func_020c9be0();
                        entry->containerDirectoryIndex_ = subdirIdx;
                        strcpy(entry->outerFilename_, innerDirPtr + subdirLength);
                        success = true;
                        goto BreakOutOfLoopSuccess;
                    }
                }
                success = false;
            }
        BreakOutOfLoopSuccess:
            if (success)
            {
                newID = s_loaderInstanceData.taskCounter_;
                numPendingTasks_++;
                s_loaderInstanceData.taskCounter_ = (newID + 1) & 0x3ff;
                flags_78c_0_ = false;
            }
        }
    }
    UnlockResourceMutex();
    return newID;
}

int BackgroundLoader::QueueOverlayTask(unsigned int id, bool load)
{
    int newID = -1;
    LockResourceMutex();
    if (numPendingTasks_ >= 24)
        func_020c9be0();
    else
    {
        BackgroundLoader::Task* entry = &queuedTasks_[numPendingTasks_];
        entry->taskID_ = s_loaderInstanceData.taskCounter_;
        entry->status_ = BackgroundLoader::TaskStatus_Unallocated;
        entry->fileLengthOrOverlayID_ = id;
        entry->type_ = (load ? BackgroundLoader::TaskType_LoadOverlay : BackgroundLoader::TaskType_UnloadOverlay);

        newID = s_loaderInstanceData.taskCounter_;
        numPendingTasks_++;
        s_loaderInstanceData.taskCounter_ = (newID + 1) & 0x3ff;
        flags_78c_0_ = false;
    }
    UnlockResourceMutex();
    return newID;
}

int BackgroundLoader::QueueLoadFile(const char* filename, SafeAllocator* alloc)
{  
    return QueueFileTask(filename, TaskType_LoadFileDefault, NULL, alloc);
}

int BackgroundLoader::QueueLoadGP1(const char* filename, SafeAllocator* alloc)
{
    return QueueFileTask(filename, TaskType_LoadGP1, NULL, alloc);
}

int BackgroundLoader::QueueLoadFileInGP2(const char* gp2, const char* innerFile, SafeAllocator* alloc)
{
    return QueueFileTask(gp2, TaskType_LoadFromGP2, innerFile, alloc);
}

int BackgroundLoader::QueueLoadOverlay(unsigned int id)
{
    return QueueOverlayTask(id, true);
}

void BackgroundLoader::AddFence()
{
    LockResourceMutex();
    if (numPendingTasks_ > 0)
    {
        if (numPendingTasks_ >= 24)
            func_020c9be0();
        else
        {
            BackgroundLoader::Task* task = &queuedTasks_[numPendingTasks_];
            task->ZeroInitialize();
            task->taskID_ = -1;
            task->status_ = BackgroundLoader::TaskStatus_Fence;
            numPendingTasks_++;
            flags_78c_0_ = false;
        }
    }
    UnlockResourceMutex();
}

int BackgroundLoader::GetTaskStatus(int taskID)
{
    int status = -1;
    LockResourceMutex();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            if (taskID != pTask->taskID_)
                continue;
            if (pTask->status_ == BackgroundLoader::TaskStatus_Complete)
                status = 1;
            else if (pTask->status_ == BackgroundLoader::TaskStatus_DecompressionFailed)
                status = -1;
            else if (pTask->status_ == BackgroundLoader::TaskStatus_LoadFileFailed)
                status = -1;
            else
                status = 0;
            break;
        }
    }
    UnlockResourceMutex();
    return status;
}

int BackgroundLoader::GetFlag0()
{
    return flags_78c_0_;
}

int BackgroundLoader::GetDetailedTaskStatus(int taskID)
{
    int status = -1;
    LockResourceMutex();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            if (taskID != pTask->taskID_)
                continue;
            status = pTask->status_;
            break;
        }
    }
    UnlockResourceMutex();
    return status;
}

void BackgroundLoader::GetLoadedFileByID(int id, void **outPtr, unsigned int *outLength)
{
    LockResourceMutex();
    *outPtr = NULL;
    *outLength = 0;
    if (id >= 0)
    {
        BackgroundLoader::Task* pTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            if (id != pTask->taskID_)
                continue;
            
            *outPtr = pTask->pFileData;
            *outLength = pTask->fileLengthOrOverlayID_;
            break;
        }
    }
    UnlockResourceMutex();
}

int BackgroundLoader::GetLoadedFileByName(const char *filename, void **outPtr, unsigned int *outLength)
{
    int taskID = -1;
    LockResourceMutex();
    *outPtr = NULL;
    *outLength = 0;
    BackgroundLoader::Task* pTask = &queuedTasks_[0];
    for (int i = 0; i < numPendingTasks_; i++, pTask++)
    {
        char fullname[80];
        pTask->GetFullFilename(fullname);

        if (strcmp(fullname, filename) == 0)
        {
            if (pTask->status_ == BackgroundLoader::TaskStatus_Complete)
            {
                *outPtr = pTask->pFileData;
                *outLength = pTask->fileLengthOrOverlayID_;
            }
            taskID = pTask->taskID_;
            break;
        }
    }
    UnlockResourceMutex();
    return taskID;
}

int BackgroundLoader::GetLoadedFileInArchive(const char *outerFile, const char* innerFile, void **outPtr, unsigned int *outLength)
{
    int taskID = -1;
    LockResourceMutex();
    *outPtr = NULL;
    *outLength = 0;
    BackgroundLoader::Task* pTask = &queuedTasks_[0];
    for (int i = 0; i < numPendingTasks_; i++, pTask++)
    {
        char fullname[80];
        pTask->GetFullFilename(fullname);

        if (strcmp(fullname, outerFile) == 0 && strcmp(pTask->innerFilename_, innerFile) == 0)
        {
            if (pTask->status_ == BackgroundLoader::TaskStatus_Complete)
            {
                *outPtr = pTask->pFileData;
                *outLength = pTask->fileLengthOrOverlayID_;
            }
            taskID = pTask->taskID_;
            break;
        }
    }
    UnlockResourceMutex();
    return taskID;
}

void BackgroundLoader::MaybeReset()
{
    LockResourceMutex();
    BackgroundLoader::Task* pTask = &queuedTasks_[0];
    for (int i = 0; i < numPendingTasks_; i++, pTask++)
    {
        FreeScratchSpace(&pTask->scratchAlloc_);
        pTask->ZeroInitialize();
    }

    numPendingTasks_ = 0;
    rightmostAllocationByte_ = 0;
    processLockBits_ = 0;
    flags_78c_1_ = false;
    flags_78c_0_ = true;
    flags_78c_2_ = true;

    UnlockResourceMutex();
}

void BackgroundLoader::MaybeFreeAllocations()
{
    if (numPendingTasks_ == 0)
        return;

    LockResourceMutex();
    BackgroundLoader::Task* pTask = &queuedTasks_[0];
    for (int i = 0; i < numPendingTasks_; i++, pTask++)
    {
        switch (pTask->status_)
        {
        case BackgroundLoader::TaskStatus_Complete:
        {
            FreeScratchSpace(&pTask->scratchAlloc_);
            if (pTask->externalAllocator_ != NULL)
            {
                pTask->externalAllocator_->Free(pTask->pFileData);
            }
            pTask->pFileData = NULL;
            pTask->fileLengthOrOverlayID_ = 0;
            break;
        }
        case BackgroundLoader::TaskStatus_Fence:
            continue; 
        }
        pTask->status_ = BackgroundLoader::TaskStatus_Unallocated;
    }
    reader_.Abort();
    flags_78c_0_ = false;
    flags_78c_2_ = true;
    UnlockResourceMutex();
}

void BackgroundLoader::RemoveTask(int taskID)
{
    if (taskID < 0)
        return;

    LockResourceMutex();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            if (taskID != pTask->taskID_)
                continue;
            
            FreeScratchSpace(&pTask->scratchAlloc_);
            pTask->ZeroInitialize();

            // Shift tasks on the right one space to the left
            BackgroundLoader::Task* pLaterTask = &queuedTasks_[i];
            for (int j = i + 1; j < numPendingTasks_; j++, pLaterTask++)
            {
                *pLaterTask = *(pLaterTask + 1);
            }
            // Now pLaterTask points to index numPendingTasks-1, i.e. 1 past the new end
            pLaterTask->ZeroInitialize();
            numPendingTasks_--;

            // All tasks at the front before one of status 5 can be removed.
            int numToRemoveFromFront = 0;
            BackgroundLoader::Task* qTask = &queuedTasks_[0];
            for (int j = 0; j < numPendingTasks_; j++, qTask++)
            {
                if (qTask->status_ != BackgroundLoader::TaskStatus_Fence)
                    break;
                numToRemoveFromFront++;
            }
            if (numToRemoveFromFront > 0)
            {
                BackgroundLoader::Task* qTask = &queuedTasks_[0];
                numPendingTasks_ -= numToRemoveFromFront;
                for (int k = 0; k < numPendingTasks_; k++, qTask++)
                {
                    *qTask = *(qTask + numToRemoveFromFront);
                }
                rightmostAllocationByte_ = 0;
                flags_78c_1_ = false;
            }
            break;
        }
    }
    UnlockResourceMutex();
}

// here goes BackgroundLoader::Task::operator=
// (the linker will remove duplicates in overlays)

int BackgroundLoader::GetNumQueuedTasks()
{
    return numPendingTasks_;
}

bool BackgroundLoader::GetTaskFilename(int taskID, char* outBuffer)
{
    bool success = false;
    LockResourceMutex();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            if (taskID != pTask->taskID_)
                continue;

            pTask->GetFullFilename(outBuffer);
            success = true;
            break;
        }
    }
    UnlockResourceMutex();
    return success;
}

void* BackgroundLoader::AllocateInScratchSpace(BackgroundLoader::Task::ScratchSpaceAllocation* output, unsigned int filesize)
{
    BackgroundLoader::Task::ScratchSpaceAllocation* orderedBlocks[24];
    unsigned int* returnAddress = NULL;

    LockResourceMutex();
    if (filesize != 0)
    {
        unsigned int requiredBytesRounded = (filesize + 3) & ~3;
        // interpretation of this is still pretty shaky
        unsigned int leftoverCapacity = scratchSpaceSize_ - scratchRightArenaUsage_;
        
        unsigned int initialNumBlocks = 0;
        unsigned int initialNumUsedWords = 0;
        unsigned int requiredWords = requiredBytesRounded >> 2;        

        // Populate the ordered pointer list
        BackgroundLoader::Task* pTask = &queuedTasks_[0];
        for (unsigned int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            if (pTask->scratchAlloc_.numWords_ == 0)
                continue;

            // insertion sort (later entries get shifted right)
            BackgroundLoader::Task::ScratchSpaceAllocation** shiftPtr = &orderedBlocks[initialNumBlocks];
            for (int j = initialNumBlocks; j != 0; j--, shiftPtr--)
            {
                if (pTask->scratchAlloc_.firstWord_ > shiftPtr[-1]->firstWord_)
                    break;
                
                shiftPtr[0] = shiftPtr[-1];
            }
            // perform the insertion
            shiftPtr[0] = &pTask->scratchAlloc_;
            initialNumBlocks++;
            initialNumUsedWords += pTask->scratchAlloc_.numWords_;
        }

        // necessary but not sufficient condition: there's collectively enough
        // space available. Now we'll check if there's a single free block big enough
        if (initialNumUsedWords + requiredWords <= (leftoverCapacity >> 2))
        {
            int allocationPosition = -1;

            if (initialNumBlocks == 0)
                allocationPosition = 0;
            else
            {
                BackgroundLoader::Task::ScratchSpaceAllocation** searchPtr = &orderedBlocks[0];
                if (requiredWords <= orderedBlocks[0]->firstWord_)
                    allocationPosition = 0;
                
                if (allocationPosition < 0)
                {
                    for (int i = 1; i < initialNumBlocks; i++, searchPtr++)
                    {
                        // Test to see if we can put it between searchPtr[0]
                        // and searchPtr[1] (that's why we start i from 1).
                        // If so, place it right after the end of searchPtr[0]
                        if (requiredWords + (searchPtr[0]->firstWord_ + searchPtr[0]->numWords_) <= searchPtr[1]->firstWord_)
                        {
                            allocationPosition = searchPtr[0]->firstWord_;
                            allocationPosition += searchPtr[0]->numWords_;
                            break;
                        }
                    }
                }

                // If we still didn't find a spot, try at the end
                // (searchPtr[0] is now the last entry in the sorted list)
                if (allocationPosition < 0)
                {
                    // this makes me think leftover capacity is the amount on the
                    // right of the scratch space
                    if (requiredWords + (searchPtr[0]->firstWord_ + searchPtr[0]->numWords_) <= (leftoverCapacity >> 2))
                    {
                        allocationPosition = searchPtr[0]->firstWord_;
                        allocationPosition += searchPtr[0]->numWords_;
                    }
                }
            }

            if (allocationPosition >= 0)
            {
                output->firstWord_ = allocationPosition;
                output->numWords_ = requiredWords;
                returnAddress = ((unsigned int*)scratchSpace_) + allocationPosition;
                RefreshCounters();
            }
        }
    }
    UnlockResourceMutex();
    return returnAddress;
}

bool BackgroundLoader::FreeScratchSpace(Task::ScratchSpaceAllocation* block)
{
    bool success = false;
    LockResourceMutex();
    if (block->numWords_ != 0)
    {
        block->firstWord_ = 0;
        block->numWords_ = 0;
        success = true;
        RefreshCounters();
    }
    UnlockResourceMutex();
    return success;
}

void BackgroundLoader::RefreshCounters()
{
    rightmostAllocationByte_ = 0;
    BackgroundLoader::Task* pTask = queuedTasks_;
    for (unsigned int i = 0; i < numPendingTasks_; i++, pTask++)
    {
        if (pTask->scratchAlloc_.numWords_ == 0)
            continue;
        unsigned int sectionEndWord = pTask->scratchAlloc_.firstWord_ + pTask->scratchAlloc_.numWords_;
        int newVal = rightmostAllocationByte_;
        if (newVal <= sectionEndWord * 4)
            newVal = sectionEndWord * 4;
        rightmostAllocationByte_ = newVal;
    }
    unsigned int unk = rightmostAllocationByte_ + scratchRightArenaUsage_;
    if (unk > unknown_120_)
        unknown_120_ = unk;
}
