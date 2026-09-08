#pragma once

#include "std_library_functions.h"

enum VRAMRegion
{
    VRAMRegion_TexturePalette = 0,
    VRAMRegion_TextureImage = 1,
    VRAMRegion_MainBGExtendedPalette = 2,
    VRAMRegion_MainBG = 3,
    VRAMRegion_SubBGExtendedPalette = 4,
    VRAMRegion_SubBG = 5,
    VRAMRegion_MainObjExtendedPalette = 6,
    VRAMRegion_MainObj = 7,
    VRAMRegion_SubObjExtendedPalette = 8,
    VRAMRegion_SubObj = 9
};

enum VRAMSubregion
{
    VRAMSubregion_TexturePalette = 0,
    VRAMSubregion_TextureImage,
    VRAMSubregion_MainBGStandardPalette,
    VRAMSubregion_MainBG0Screen,
    VRAMSubregion_MainBG0Character,
    VRAMSubregion_MainBG1Screen,
    VRAMSubregion_MainBG1Character,
    VRAMSubregion_MainBG2Screen,
    VRAMSubregion_MainBG2Character,
    VRAMSubregion_MainBG3Screen,
    VRAMSubregion_MainBG3Character,
    VRAMSubregion_SubBGStandardPalette,
    VRAMSubregion_SubBG0Screen,
    VRAMSubregion_SubBG0Character,
    VRAMSubregion_SubBG1Screen,
    VRAMSubregion_SubBG1Character,
    VRAMSubregion_SubBG2Screen,
    VRAMSubregion_SubBG2Character,
    VRAMSubregion_SubBG3Screen,
    VRAMSubregion_SubBG3Character,
    VRAMSubregion_MainObjStandardPalette,
    VRAMSubregion_MainObj,
    VRAMSubregion_SubObjStandardPalette,
    VRAMSubregion_SubObj
};

// Uses a 20K buffer at 0x0214e628 (usa) to hold VRAM-related things
// before they need to be loaded into memory
// sizeof == 0x44.
class VRAMStagingManager
{
public:
    class StagingSpaceAllocation
    {
    public:
        unsigned short start_;
        unsigned short size_;
        // bit 0: is allocated
        // I don't see any other flags being used!
        unsigned short flags_;

        // usa: func_01ff8ab4
        void SetFlagBitValue(int bit, bool value);
    };

    // There is an array of 0x100 of these treated as a circular
    // queue
    class Task
    {
    public:
        // not to be confused with the index (in the array), this is just
        // a rolling counter in the staging manager so you can refer back
        // to a task after it's been created
        int taskID_ : 12;
        int destinationOffset_ : 20; // divided by 4
        int subregion_ : 6; // VRAMSubregion
        int region_ : 6; // VRAMRegion
        int numWordsToCopy_ : 20;
        int stagingAllocIndex_ : 8;
        // bit 0 = in the queue, bit 1 = complete/cancelled, bit 2 = high priority
        int flags_ : 4; 
        int wordsCopied_ : 20;
        const void* copySource_;
    
        // usa: func_01ff8e40
        void Reset();
        // usa: func_01ff8ebc
        void SetFlagBitValue(int bit, bool value);
    };
    
    // pending tasks are dispatched one region at a time, these are used
    // to group said tasks together
    struct CommonVRAMRegionTaskSet
    {
        // points to a buffer also held in ITCM
        unsigned char* pendingTaskIndices_;
        unsigned short maxNumTasks_;
        // 0 = regular, 1 = high priority
        unsigned short numTasksPerPriority_[2];
        unsigned short numTasks_;

        // usa: func_01ff96ac
        // Only resets the counters as no need to clear the task index
        // buffer (when the counters increase again, its contents will
        // be overwritten)
        void Reset();
    };

public:
    unsigned short queueFront_;
    unsigned short queueEnd_;
    unsigned short historicalMaxQueueLength_;
    unsigned short maybeMaxHistorialNumSize6_;
    unsigned short maybeMaxHistoricalSize6MemoryUse_;
    // annoyingly this is referenced as both signed and unsigned, and
    // even in the same function 10 lines apart, so needs some casting
    short stagedTaskCounter_;
    unsigned short banksInUse_[10];
    char unk_20[0x3c - 0x20]; // previous array might be of size 24 instead
    // refcounting by <<1 | 1 and >>1.
    // While nonzero, texture image and texture palette copying tasks will
    // not execute. (All other tasks will execute as normal)
    unsigned int textureLockMask_; 
    unsigned char frameBufferIndex_; // might be bool

    // usa: func_01ff8810
    void ZeroInitialize();

    // usa: func_01ff8850
    Task* GetTaskByID(int id);
    // usa: func_01ff88c4
    StagingSpaceAllocation* GetStagingSpaceAllocation(const void* allocation);

    // usa: func_01ff891c
    void* AllocateInStagingSpace(unsigned int length);
    // usa: func_01ff8adc
    bool FreeStagingSpaceAllocation(const void* allocation);
    // usa: func_01ff8b08
    bool FreeStagingSpaceAllocationByIndex(unsigned int idx);
    // usa: func_01ff8b48
    // if allocateStagingSpace is set to true, then the provided data will
    // be copied (either by memcpy or DMA) into the staging buffer
    int Stage(VRAMSubregion subregion, const void* data, unsigned int offset,
        unsigned int length, bool highPriority, bool allocateStagingSpace);
    // usa: func_01ff8f00
    bool CancelTaskByID(int id);
    // usa: func_01ff8f38
    void CancelAllTasksInRegion(VRAMRegion region);
    // usa: func_01ff8fb8
    // any pending tasks writing to a subset of the region specified in the
    // arguments to this function can be cancelled as they won't be seen.
    // (Note that partial overlap of write regions is not sufficient here)
    void CancelOverwrittenTasks(VRAMSubregion subregion, unsigned int offset, unsigned int length);
    // usa: func_01ff905c
    void CancelAllTasks();
    // usa: func_01ff9098
    void UpdateBanks();
    // usa: func_01ff91ac
    void SendReadyDataToVRAM();
}; 

// usa: func_01ff96c0
// Just calls SendReadyDataToVRAM() on the instance provided in the
// userdata. I think this is written so it can be passed as a callback
// to e.g. an interrupt handler
void SendStagedVRAMDataToVRAM(void* vramStagingManagerUserdata);