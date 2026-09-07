#pragma once

#include "ExtendedNitroVM.h"
#include "System/ProcessorContext.h"
#include "Memory/SafeAllocator.h"

// sizeof == 0x794 == 1940 bytes. This struct is passed as the first
// parameter to various functions in the 0202fxxx range, and an instance
// of it is constructed in overlay 33's bss section (at 0x022a2a2c, usa).
// It does stuff related to file loading, a layer on top of ExtendedNitroVM.
// A pointer to this instance is then held at 0x02104308 (usa)
struct BackgroundLoader
{
    // We might want an inline constructor here

    // sizeof == 0x44 == 68 bytes
    struct Task
    {
        char outerFilename_[0x18];
        char innerFilename_[0x18];
        short taskID_; // unrelated to nitro file ID or cache index
        char status_ : 4;
        char type_ : 4;
        unsigned char containerDirectoryIndex_; // this is indexing directories within data/
        SafeAllocator* externalAllocator_;
        void* pFileData;
        unsigned int fileLengthOrOverlayID_;
        struct ScratchSpaceAllocation {
            unsigned short firstWord_;
            unsigned short numWords_;
        }; 
        ScratchSpaceAllocation scratchAlloc_;

        // usa: func_0202f700
        void ZeroInitialize();
        // usa: func_0202f760
        bool GetFullFilename(char* outBuffer);
    };

    enum TaskStatus
    {
        TaskStatus_Invalid = -1,
        TaskStatus_Unallocated = 0,
        TaskStatus_InFlight = 1,
        TaskStatus_Complete = 2,
        TaskStatus_DecompressionFailed = 3,
        TaskStatus_LoadFileFailed = 4,
        // if a fence task is in the queue, no later tasks will be processed
        // until all prior tasks have been removed / freed. Not sure why this
        // is a status and not a task type, but oh well
        TaskStatus_Fence = 5,
    };

    enum TaskType
    {
        TaskType_LoadFileDefault = 0, // seems to be regular file
        TaskType_LoadGP1 = 1, // only used in one place
        TaskType_LoadFromGP2 = 2,
        TaskType_LoadOverlay = 3, // never called in practice?
        TaskType_UnloadOverlay = 4, // never called in practice?
    };

    // I wanted to implement this as a pointer to a function pointer, but it
    // didn't match, whereas virtual stuff did. This is offset 0.
    virtual int Process() = 0;

    // points to 0x022a2a08, which in turn points to the function
    // func_ov_033_022a21a8. The context's execution repeatedly calls this function
    ProcessorContext context_;
    ExtendedNitroVM reader_;
    // The scratch space is used for two purposes. On the left, we have block-based
    // allocation, roughly like HPXEAllocator, used for files loaded without
    // a SafeAllocator, and on the right we have an arena that grows leftward
    // used by the processing thread to store GP2-related metadata. (Note that
    // only one archive's worth of metadata is ever in the arena at a time)
    void* scratchSpace_;
    volatile unsigned int scratchSpaceSize_;
    volatile int rightmostAllocationByte_;
    volatile unsigned int scratchRightArenaUsage_;
    // I think this is just for benchmarking / debugging, I don't see any reads
    // from it. It holds the maximum recorded value of (rightmostAllocationByte
    // + scratchRightArenaUsage), i.e. maximum historical memory usage.
    // It's initialized to 0x10000 (64k, 1/3 of the size used in practice).
    int unknown_120_;
    volatile int numPendingTasks_;
    Task queuedTasks_[24];
    // like a refcount, but works by going << 1 and | 1 to increment, >> 1 to decrement.
    // While nonzero, the processing thread will not do any tasks. This is useful
    // if e.g. you want to load a file into the same scratch space (see e.g.
    // DetailedTreasureMapData::LoadLegacyBossStats())
    volatile unsigned int processLockBits_;
    volatile int flags_78c_0_ : 1;
    volatile int flags_78c_1_ : 1;
    // might be something like 'scratch space allocation is up to date'
    volatile int flags_78c_2_ : 1;
    volatile int flags_78c_3_ : 1;
    volatile int flagMaybeGP2OperationInFlight_ : 1;
    int unknown_790_;

    // usa: func_0202f798
    static BackgroundLoader* GetInstance();
    // usa: func_0202f7a8
    static void FreeAllocationsGlobal();
    // usa: func_0202f7c8
    static void AddLockGlobal();
    // usa: func_0202f7e8
    static void RemoveLockGlobal();

    // usa: func_0202f808
    void InitializeOrReset();
    // usa: func_0202f894
    void Populate(void* scratchSpace, unsigned int scratchSize, int relativePrio);
    // usa: func_0202f920
    void MaybeWaitIdle();
    // usa: func_0202f984
    void AddLock();
    // usa: func_0202f9b4
    void RemoveLock();
    // usa: func_0202fa00
    void RemoveAllLocks();
    // usa: func_0202fa38
    // If alloc is not null, the file will be loaded into dynamically allocated
    // memory from the allocator, otherwise, the scratch space will be used
    int QueueFileTask(const char* filename, int type, const char* innerFile, SafeAllocator* alloc);
    // usa: func_0202fc38
    // Queues a load or unload of an overlay.
    int QueueOverlayTask(unsigned int id, bool load);

    // usa: func_0202fcfc
    int QueueLoadFile(const char* filename, SafeAllocator* alloc);
    // usa: func_0202fd14
    int QueueLoadGP1(const char* filename, SafeAllocator* alloc);
    // usa: func_0202fd2c
    int QueueLoadFileInGP2(const char* gp2, const char* innerFile, SafeAllocator* alloc);
    // usa: func_0202fd44
    int QueueLoadOverlay(unsigned int id);
    // usa: func_0202fd54
    void AddFence();

    // usa: func_0202fdd0
    // -1 = failed, 0 = underway/queued maybe?, 1 = successfully completed
    int GetTaskStatus(int taskID);
    // usa: func_0202fe58
    int GetFlag0();
    // usa: func_0202fe68
    int GetDetailedTaskStatus(int taskID);

    // usa: func_0202fec8
    void GetLoadedFileByID(int taskID, void** outPtr, unsigned int* outLength);
    // usa: func_0202ff34
    // returns the ID of the task that loaded this file, or -1 if not loaded.
    int GetLoadedFileByName(const char* name, void** outPtr, unsigned int* outLength);
    // usa: func_0202ffd8
    // returns the ID of the task that loaded this file, or -1 if not loaded.
    int GetLoadedFileInArchive(const char* archive, const char* innerFile, void** outPtr, unsigned int* outLength);

    // usa: func_02030090
    void MaybeReset();
    // usa: func_02030110
    void MaybeFreeAllocations();
    // usa: func_020301c8
    void RemoveTask(int taskID);

    // implicitly created: func_02030310 = Task::operator=(const Task&)

    // usa: func_02030390
    int GetNumQueuedTasks();
    // usa: func_02030398
    // returns true if a task of the specified id was found
    bool GetTaskFilename(int taskID, char* outBuffer);

protected:
    // usa: func_02030400
    void* AllocateInScratchSpace(Task::ScratchSpaceAllocation* output, unsigned int allocSize);
    // usa: func_02030584
    bool FreeScratchSpace(Task::ScratchSpaceAllocation* block);
    // usa: func_020305c8
    void RefreshCounters();
};