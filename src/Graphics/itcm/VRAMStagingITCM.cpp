#include "Graphics/itcm/VRAMStaging.h"
#include "System/VRAM.h"
#include "System/LoadToVRAM.h"
#include "System/Cache.h"
#include "System/Graphics.h"
#include "Resource/ResourceMutex.h"
#include "std_library_functions.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_020ee694 data_020ee7a0
#define data_020ee6b0 data_020ee7bc
#define data_020ee6ba data_020ee7c6
#define data_020ee6d0 data_020ee7dc
#define data_020ee6e8 data_020ee7f4

#define func_0202ae18 func_0202a9d0
#define func_0202ae24 func_0202a9dc
#define func_0202b2f0 func_0202aea0
#define func_0205e57c func_0205f868
#define func_020d84f8 func_020d9e5c
#define func_020d8704 func_020da068

#endif

#define DATA_027fffa8 (*(unsigned short*)0x027fffa8)

extern "C"
{
    void* func_0202ae18();
    void* func_0202ae24();
    void func_0202b2f0(void*);
    void func_0205e57c(void*);

    // zero memory and flush cache
    void func_020d84f8(void*, unsigned int);

    // gets a byte at 0x0214e4a0
    int func_020d8704();
}

// seems to be one per VRAMRegion
extern VRAMStagingManager::CommonVRAMRegionTaskSet g_vramStagingRegionalTaskSets[10];
extern VRAMStagingManager::StagingSpaceAllocation g_vramStagingAllocations[0x80];
extern VRAMStagingManager::Task g_vramStagingTaskQueue[0x100];
extern unsigned char g_vramStagingBuffer[0x5000];

#define TASK_FLAG_VALID_TASK 0
#define TASK_FLAG_COMPLETE 1
#define TASK_FLAG_HIGH_PRIORITY 2

#define STAGING_SPACE_ALLOC_FLAG_ALLOCATED 0

#define STAGING_BUFFER_SIZE 0x5000

// elements {262, 212}. 262 is number of scanlines, not sure about the 212
extern const short data_020ee694[];

// maps VRAMRegion enum to {0 = main, 1 = sub}
extern const char data_020ee6b0[];

// holds pairs (VRAMRegion, priority) and pending tasks are executed in 
// this order. When priority = -1 (which is the case for all except
// VRAMRegion_TextureImage), both regular and high priority are handled jointly
extern const char data_020ee6ba[];

// maps VRAMSubregion to VRAMRegion
extern const char data_020ee6d0[];

// function pointers to get the main/sub screen master brightness (-16...16)
// note -16 means fully black and 16 means fully white
extern int (*g_masterBrightnessFunctions[2])();

// function pointers to get the currently active components (BG0, .., BG3, Obj)
// of the main/sub engine
extern int (*g_activeComponentFunctions[2])();

// maps VRAMSubregion to a bitmask of layers
// (BG0 = 1, BG1 = 2, BG2 = 4, BG3 = 8, Obj = 16)
// corresponding to which ones the subregion's data can influence
extern const char data_020ee6e8[];

// maps VRAMSubregion to the function pointer for loading data to said
// subregion
extern void (*g_vramSubregionLoadFunctions[24])(const void*, unsigned int, unsigned int);

bool IsSafeToModifySubregion(int subregion)
{
    int mainOrSubScreen = data_020ee6b0[data_020ee6d0[subregion]];
    bool safe = true;
    
    int absBrightness = abs(g_masterBrightnessFunctions[mainOrSubScreen]());

    // if the brightness is +16 or -16 the whole screen is white or black
    // so it's safe to mess with vram no matter what
    if (absBrightness < 16)
    {
        int activeComponents = g_activeComponentFunctions[mainOrSubScreen]();
        if (data_020ee6e8[subregion] & activeComponents)
            safe = false;
    }

    return safe;
}

void VRAMStagingManager::ZeroInitialize()
{
    queueFront_ = 0;
    queueEnd_ = 0;
    historicalMaxQueueLength_ = 0;
    maybeMaxHistorialNumSize6_ = 0;
    maybeMaxHistoricalSize6MemoryUse_ = 0;
    stagedTaskCounter_ = 0;
    func_020d84f8(banksInUse_, 0x30);
    textureLockMask_ = 0;
    frameBufferIndex_ = 0;
}

VRAMStagingManager::Task* VRAMStagingManager::GetTaskByID(int id)
{
    Task* entry = &g_vramStagingTaskQueue[queueFront_];
    for (unsigned int i = 0; i < 0x100; i++, entry++)
    {
        if (entry >= &g_vramStagingTaskQueue[0x100])
            entry = &g_vramStagingTaskQueue[0];

        unsigned int flags = entry->flags_;
        if (!(flags & (1 << TASK_FLAG_VALID_TASK)))
            break;

        if (id != entry->taskID_)
            continue;

        if (flags & (1 << TASK_FLAG_COMPLETE))
            entry = NULL;
        return entry;
    }

    return NULL;
}

VRAMStagingManager::StagingSpaceAllocation*
VRAMStagingManager::GetStagingSpaceAllocation(const void* allocation)
{
    uintptr_t targetOffset = (uintptr_t)allocation - (uintptr_t)&g_vramStagingBuffer[0];
    if (targetOffset >= STAGING_BUFFER_SIZE)
        return NULL;
    StagingSpaceAllocation* candidate = &g_vramStagingAllocations[0];
    for (unsigned int i = 0; i < 0x80; i++, candidate++)
    {
        if ((candidate->flags_ & (1 << STAGING_SPACE_ALLOC_FLAG_ALLOCATED))
            && candidate->start_ == targetOffset)
            return candidate;
    }
    return NULL;
}

void* VRAMStagingManager::AllocateInStagingSpace(unsigned int length)
{
    StagingSpaceAllocation* sortedList[0x80];
    unsigned char* allocation = NULL;
    unsigned int numEntries = 0;
    unsigned int totalSpaceUsed = 0;
    StagingSpaceAllocation* firstFree = NULL;    

    // Insertion sort to generate a sorted list of active allocations by
    // start address, and also find a free space in the array to store
    // details of the new allocation 
    StagingSpaceAllocation* insertionPtr = &g_vramStagingAllocations[0];
    for (unsigned int i = 0; i < 0x80; i++, insertionPtr++)
    {
        if (insertionPtr->flags_ & (1 << STAGING_SPACE_ALLOC_FLAG_ALLOCATED))
        {
            StagingSpaceAllocation** movePtr = &sortedList[numEntries];
            for (int idx = numEntries; idx != 0; idx--)
            {
                if (insertionPtr->start_ > movePtr[-1]->start_)
                    break;
                movePtr[0] = movePtr[-1];
                movePtr--;
            }
            movePtr[0] = insertionPtr;
            numEntries++;
            totalSpaceUsed += insertionPtr->size_;
        }
        else
        {
            if (firstFree == NULL)
                firstFree = insertionPtr;
        }
    }
    
    if (firstFree != NULL)
    {
        unsigned int alignedLength = (length + 3) & ~3;
        if (totalSpaceUsed + alignedLength <= STAGING_BUFFER_SIZE)
        {
            int chosenOffset = -1;
            if (numEntries == 0)
                chosenOffset = 0;
            else
            {
                StagingSpaceAllocation** searchPtr = &sortedList[0];
                if (alignedLength <= searchPtr[0]->start_)
                    chosenOffset = 0;
                
                if (chosenOffset < 0)
                {
                    for (int i = 1; i < numEntries; i++, searchPtr++)
                    {
                        unsigned int nextFreeOffset = searchPtr[0]->start_ + searchPtr[0]->size_;
                        if (alignedLength + nextFreeOffset <= searchPtr[1]->start_)
                        {
                            chosenOffset = searchPtr[0]->start_;
                            chosenOffset += searchPtr[0]->size_;
                            break;
                        }
                    }
                }

                if (chosenOffset < 0)
                {
                    unsigned int nextFreeOffset = searchPtr[0]->start_ + searchPtr[0]->size_;
                    if (alignedLength + nextFreeOffset <= STAGING_BUFFER_SIZE)
                    {
                        chosenOffset = searchPtr[0]->start_;
                        chosenOffset += searchPtr[0]->size_;
                    }
                }
            }

            if (chosenOffset >= 0)
            {
                firstFree->start_ = chosenOffset;
                firstFree->size_ = alignedLength;
                totalSpaceUsed += alignedLength;
                numEntries++;
                firstFree->SetFlagBitValue(STAGING_SPACE_ALLOC_FLAG_ALLOCATED, true);
                allocation = &g_vramStagingBuffer[chosenOffset];
            }
        }
    }

    if (maybeMaxHistorialNumSize6_ < numEntries)
        maybeMaxHistorialNumSize6_ = numEntries;

    if (maybeMaxHistoricalSize6MemoryUse_ < totalSpaceUsed)
        maybeMaxHistoricalSize6MemoryUse_ = totalSpaceUsed;
    return (void*)allocation;
}

void VRAMStagingManager::StagingSpaceAllocation::SetFlagBitValue(int bit, bool value)
{
    unsigned short mask = 1 << bit;
    if (value)
        flags_ |= mask;
    else
        flags_ &= ~mask;
}

bool VRAMStagingManager::FreeStagingSpaceAllocation(const void* allocation)
{
    bool success = false;
    StagingSpaceAllocation* entry = GetStagingSpaceAllocation(allocation);
    if (entry != NULL)
    {
        entry->SetFlagBitValue(STAGING_SPACE_ALLOC_FLAG_ALLOCATED, false);
        success = true;
    }
    return success;
}

bool VRAMStagingManager::FreeStagingSpaceAllocationByIndex(unsigned int idx)
{
    bool success = false;
    if (idx < 0x80)
    {
        StagingSpaceAllocation* entry = &g_vramStagingAllocations[idx];
        if (entry->flags_ & (1 << STAGING_SPACE_ALLOC_FLAG_ALLOCATED))
        {
            entry->SetFlagBitValue(STAGING_SPACE_ALLOC_FLAG_ALLOCATED, false);
            success = true;
        }
    }
    return success;
}

int VRAMStagingManager::Stage(VRAMSubregion subregion, const void* data, unsigned int offset,
    unsigned int length, bool bit2Flag, bool allocateStagingSpace)
{
    int newTaskID = -1;
    if (subregion < 24u && data != NULL && length != 0)
    { 
        CancelOverwrittenTasks(subregion, offset, length);
        int region = data_020ee6d0[subregion];
        if (IsSafeToModifySubregion(subregion))
        {
            textureLockMask_ <<= 1;
            textureLockMask_ |= 1;
            switch (region)
            {
            case VRAMRegion_TexturePalette:
                MemoryMapTexturePalette();
                break;
            case VRAMRegion_TextureImage:
                MemoryMapTextureImage();
                break;
            }
            CleanInvalidateCacheRange(data, length);
            g_vramSubregionLoadFunctions[subregion](data, offset, length);
            CleanCacheRange(data, length);
            switch (region)
            {
            case VRAMRegion_TexturePalette:
                MemoryUnmapTexturePalette();
                break;
            case VRAMRegion_TextureImage:
                MemoryUnmapTextureImage();
                break;
            }
            textureLockMask_ >>= 1;
            FreeStagingSpaceAllocation(data);
        }
        else
        {
            StagingSpaceAllocation* allocSpaceData = GetStagingSpaceAllocation(data);
            Task* newTask = &g_vramStagingTaskQueue[queueEnd_];
            if (!(newTask->flags_ & (1 << TASK_FLAG_VALID_TASK))) // check for space in queue
            {
                const void* stagingSpace = data;
                if (allocateStagingSpace && allocSpaceData == NULL)
                {
                    void* allocSpace = AllocateInStagingSpace(length);
                    if (allocSpace == NULL)
                        stagingSpace = NULL; // wtf?!
                    else
                    {
                        memcpy(allocSpace, data, length);
                        allocSpaceData = GetStagingSpaceAllocation(allocSpace);
                        stagingSpace = allocSpace;
                        CleanInvalidateCacheRange(allocSpace, length);
                    }
                }
                if (stagingSpace != NULL)
                {
                    newTask->Reset();
                    newTask->taskID_ = stagedTaskCounter_;
                    short allocIndex = allocSpaceData == NULL ? -1 : allocSpaceData - g_vramStagingAllocations;
                    newTask->stagingAllocIndex_ = allocIndex;
                    newTask->subregion_ = (char)subregion;
                    newTask->region_ = (char)region;
                    newTask->copySource_ = stagingSpace;
                    
                    newTask->destinationOffset_ = offset >> 2;
                    newTask->numWordsToCopy_ = ((length + 3) & ~3) >> 2;
                    newTask->wordsCopied_ = 0;
                    newTask->SetFlagBitValue(TASK_FLAG_HIGH_PRIORITY, bit2Flag);
                    newTaskID = newTask->taskID_;
                    queueEnd_ = (queueEnd_ + 1 >= 0x100) ? 0 : (queueEnd_ + 1);
                    stagedTaskCounter_ = ((unsigned short)stagedTaskCounter_ + 1) & 0x7ff;
                    newTask->SetFlagBitValue(TASK_FLAG_VALID_TASK, true);
                }
            }
            if (newTaskID < 0 && allocSpaceData != NULL)
                allocSpaceData->SetFlagBitValue(STAGING_SPACE_ALLOC_FLAG_ALLOCATED, false);
        }
    }
    return newTaskID;
}

void VRAMStagingManager::Task::Reset()
{
    flags_ = 0;
    taskID_ = -1;
    subregion_ = -1;
    region_ = -1;
    stagingAllocIndex_ = -1;
    copySource_ = 0;
    numWordsToCopy_ = 0;
    destinationOffset_ = 0;
    wordsCopied_ = 0;
}

void VRAMStagingManager::Task::SetFlagBitValue(int bit, bool value)
{
    unsigned char mask = 1 << bit;
    if (value)
        flags_ |= mask;
    else
        flags_ &= ~mask;
}

bool VRAMStagingManager::CancelTaskByID(int id)
{
    if (id < 0)
        return false;

    bool success = false;
    Task* task = GetTaskByID(id);
    if (task != NULL)
    {
        task->SetFlagBitValue(TASK_FLAG_COMPLETE, true);
        success = true;
    }

    return success;
}

void VRAMStagingManager::CancelAllTasksInRegion(VRAMRegion region)
{
    Task* task = &g_vramStagingTaskQueue[queueFront_];
    for (unsigned int i = 0; i < 0x100; i++, task++)
    {
        if (task >= &g_vramStagingTaskQueue[0x100])
            task = &g_vramStagingTaskQueue[0];
        unsigned int flags = task->flags_;
        if (!(flags & (1 << TASK_FLAG_VALID_TASK)))
            return;
        if (!(flags & (1 << TASK_FLAG_COMPLETE)) && region == task->region_)
        {
            task->SetFlagBitValue(TASK_FLAG_COMPLETE, true);
        }
    }
}

void VRAMStagingManager::CancelOverwrittenTasks(VRAMSubregion subregion,
    unsigned int offset, unsigned int length)
{
    Task* task = &g_vramStagingTaskQueue[queueFront_];
    for (unsigned int i = 0; i < 0x100; i++, task++)
    {
        if (task >= &g_vramStagingTaskQueue[0x100])
            task = &g_vramStagingTaskQueue[0];

        unsigned int flags = task->flags_;
        if (!(flags & (1 << TASK_FLAG_VALID_TASK)))
            return;
        if (!(flags & (1 << TASK_FLAG_COMPLETE)) && subregion == task->subregion_)
        {
            if (offset <= task->destinationOffset_ * 4 &&
                (task->destinationOffset_ + task->numWordsToCopy_) * 4 <= offset + length)
            {
                task->SetFlagBitValue(TASK_FLAG_COMPLETE, true);
            }
        }
    }
}

void VRAMStagingManager::CancelAllTasks()
{
    Task* entry = &g_vramStagingTaskQueue[0];
    for (unsigned int i = 0; i < 0x100; i++, entry++)
        entry->SetFlagBitValue(TASK_FLAG_COMPLETE, true);
}

void VRAMStagingManager::UpdateBanks()
{
    for (unsigned int region = 0; region < 10; region++)
    {
        unsigned short priorBanks = banksInUse_[region];
        switch (region)
        {
        case VRAMRegion_TexturePalette:
            banksInUse_[region] = GetTexturePaletteVRAMBanks();
            break;
        case VRAMRegion_TextureImage:
            banksInUse_[region] = GetTextureImageVRAMBanks();  
            break;
        case VRAMRegion_MainBGExtendedPalette:
            banksInUse_[region] = GetMainBGExtendedPaletteVRAMBanks();
            break;
        case VRAMRegion_MainBG:
            region++; // silly register hack
            banksInUse_[region - 1] = GetMainBGVRAMBanks();
            region--;
            break;
        case VRAMRegion_SubBGExtendedPalette:
            banksInUse_[region] = GetSubBGExtendedPaletteVRAMBanks();
            break;
        case VRAMRegion_SubBG:
            banksInUse_[region] = GetSubBGVRAMBanks();
            break;
        case VRAMRegion_MainObjExtendedPalette:
            banksInUse_[region] = GetMainObjExtendedPaletteVRAMBanks();
            break;
        case VRAMRegion_MainObj:
            banksInUse_[region] = GetMainObjVRAMBanks();
            break;
        case VRAMRegion_SubObjExtendedPalette:
            banksInUse_[region] = GetSubObjExtendedPaletteVRAMBanks();
            break;
        case VRAMRegion_SubObj:
            banksInUse_[region] = GetSubObjVRAMBanks();
            break;
        default:
            continue;
        }
        if (priorBanks != banksInUse_[region])
            CancelAllTasksInRegion((VRAMRegion)region);
    }
}

void VRAMStagingManager::SendReadyDataToVRAM()
{
    // Declaring all of these here fixes stack issues
    bool mutexState;
    unsigned int groupingCounter;
    CommonVRAMRegionTaskSet* loopRegionTaskSet;
    unsigned int taskSetInnerCounter;
    const void* copySource;
    int destination;
    
    mutexState = SetResourceMutexOperational(false);
    int bVar1;
    int canOverrideCopyLimits = (DATA_027fffa8 & 0x8000) >> 15;
    
    bVar1 = frameBufferIndex_ != func_020d8704() || canOverrideCopyLimits != 0;
    if (bVar1)
        frameBufferIndex_ = func_020d8704();
    
    
    unsigned int queueHead;
    queueHead = queueFront_;
    CommonVRAMRegionTaskSet* regionSet = &g_vramStagingRegionalTaskSets[0];
    for (unsigned int i = 0; i < 10; i++, regionSet++)
        regionSet->Reset();

    bool regionsToModifyLookup[24];
    int thisFunctionTaskCount = 0;
    unsigned int numTasksInQueue = 0;
    for (unsigned int subregion = 0; subregion < 24; subregion++)
    {
        regionsToModifyLookup[subregion] = bVar1 || IsSafeToModifySubregion(subregion);
    }

    unsigned int currentQueueIndex = queueHead;
    Task* pTask = &g_vramStagingTaskQueue[currentQueueIndex];
    for (unsigned int i = 0; i < 0x100; i++, currentQueueIndex++, pTask++)
    {
        if (currentQueueIndex >= 0x100)
        {
            pTask = &g_vramStagingTaskQueue[0];
            currentQueueIndex = 0;
        }
        unsigned int flags = pTask->flags_;
        if (!(flags & (1 << TASK_FLAG_VALID_TASK)))
            break;
        numTasksInQueue++;
        if (!(flags & (1 << TASK_FLAG_COMPLETE)) && regionsToModifyLookup[pTask->subregion_])
        {
            CommonVRAMRegionTaskSet* foo;
            foo = &g_vramStagingRegionalTaskSets[pTask->region_];
            if (foo->numTasks_ < foo->maxNumTasks_)
            {
                thisFunctionTaskCount++;
                foo->pendingTaskIndices_[foo->numTasks_] = currentQueueIndex;
                int priority = (pTask->flags_ & (1 << TASK_FLAG_HIGH_PRIORITY)) ? 1 : 0;
                foo->numTasksPerPriority_[priority]++;
                foo->numTasks_++;
            }
        }
    }

    if (historicalMaxQueueLength_ < numTasksInQueue)
        historicalMaxQueueLength_ = numTasksInQueue;

    // Holds pairs of the form (VRAMRegion, priority)
    // with priority = -1 meaning do both 0 and 1 at once
    const char* groupingData = &data_020ee6ba[0];
         
    // we loop 11 times because we use the priority system
    // for texture image tasks
    for (groupingCounter = 0; groupingCounter < 11; groupingData += 2, groupingCounter++)
    {
        if (thisFunctionTaskCount <= 0)
            break;
        int region = groupingData[0];
        
        loopRegionTaskSet = &g_vramStagingRegionalTaskSets[region];
        if (loopRegionTaskSet->numTasks_ == 0)
            continue;
        if (groupingData[1] >= 0)
        {
            if (loopRegionTaskSet->numTasksPerPriority_[groupingData[1]] == 0)
                continue;
            thisFunctionTaskCount -= loopRegionTaskSet->numTasksPerPriority_[groupingData[1]];
        }
        else
            thisFunctionTaskCount -= loopRegionTaskSet->numTasks_;
        if (textureLockMask_ != 0 && (region == VRAMRegion_TexturePalette || region == VRAMRegion_TextureImage))
            continue;
        
        int maxAmountCopyableThisGroup;
        if (groupingData[1] == 0) // explicitly marked as low priority
        {
            int vcount = VCOUNT;
            // data_020ee694 is 262 for main engine or 212 for sub engine
            maxAmountCopyableThisGroup = (data_020ee694[region >= 4 ? 1 : 0] - vcount) * 0x400;
            maxAmountCopyableThisGroup = (maxAmountCopyableThisGroup + 3) & ~3;
        }  
        else
            maxAmountCopyableThisGroup = 0x00ffffff; // 16MB

        if (canOverrideCopyLimits != 0)
            maxAmountCopyableThisGroup = 0x0fffffff; // 256MB

        if (maxAmountCopyableThisGroup <= 0)
            continue;

        unsigned int amountCopiedThisGroup = 0;
        switch (region)
        {
        case VRAMRegion_TexturePalette:
            MemoryMapTexturePalette();
            break;
        case VRAMRegion_TextureImage:
            MemoryMapTextureImage();
            break;
        }

        for (taskSetInnerCounter = 0; taskSetInnerCounter < loopRegionTaskSet->numTasks_; taskSetInnerCounter++)
        {
            Task* pTask;
            // changing signedness here fixes register assignment problems
            unsigned int maxAmount = maxAmountCopyableThisGroup;
            if (maxAmount <= amountCopiedThisGroup)
                break;
            int taskIndex = loopRegionTaskSet->pendingTaskIndices_[taskSetInnerCounter];
            pTask = &g_vramStagingTaskQueue[taskIndex];
            if (pTask->flags_ & (1 << TASK_FLAG_COMPLETE))
                continue;
            if (groupingData[1] == 1 && !(pTask->flags_ & (1 << TASK_FLAG_HIGH_PRIORITY)))
                continue;

            unsigned int amountToCopyThisTask;
            amountToCopyThisTask = ((pTask->numWordsToCopy_ - pTask->wordsCopied_) * 4 + 3) & ~3;
            
            if (maxAmount <= amountToCopyThisTask + amountCopiedThisGroup)
            {
                amountToCopyThisTask = (maxAmount - amountCopiedThisGroup + 3) & ~3;
            }
            copySource = (const void*)((intptr_t)pTask->copySource_ + pTask->wordsCopied_ * 4);
            destination = (pTask->destinationOffset_ + pTask->wordsCopied_) * 4;
            CleanInvalidateCacheRange(copySource, amountToCopyThisTask);
            // e.g. LoadToTexturePalette
            g_vramSubregionLoadFunctions[pTask->subregion_](copySource, destination, amountToCopyThisTask);
            CleanCacheRange(copySource, amountToCopyThisTask);
            amountCopiedThisGroup += amountToCopyThisTask;
            pTask->wordsCopied_ += amountToCopyThisTask >> 2;
            if (pTask->wordsCopied_ < pTask->numWordsToCopy_)
                break;

            pTask->SetFlagBitValue(TASK_FLAG_COMPLETE, true);
            
        }

        switch (groupingData[0]) // region
        {
        case VRAMRegion_TexturePalette:
            MemoryUnmapTexturePalette();
            break;
        case VRAMRegion_TextureImage:
            MemoryUnmapTextureImage();
            break;
        }
    }
    
    unsigned int pos = queueHead;
    Task* completionLoopTask = &g_vramStagingTaskQueue[pos];
    for (unsigned int i = 0; i < 0x100; i++, pos++, completionLoopTask++)
    {
        if (pos >= 0x100)
        {
            completionLoopTask = &g_vramStagingTaskQueue[0];
            pos = 0;
        }
        unsigned int flags = completionLoopTask->flags_;
        if (!(flags & (1 << TASK_FLAG_VALID_TASK)) || !(flags & (1 << TASK_FLAG_COMPLETE)))
            break;
        FreeStagingSpaceAllocationByIndex(((volatile Task*)completionLoopTask)->stagingAllocIndex_);
        completionLoopTask->Reset();
        queueFront_ = pos + 1;
        if (queueFront_ >= 0x100)
            queueFront_ = 0;
    }

    void* unknownPtr1 = func_0202ae18();
    void* unknownPtr2 = func_0202ae24();
    func_0202b2f0(unknownPtr1);
    func_0205e57c(unknownPtr2);

    SetResourceMutexOperational(mutexState);
}

void VRAMStagingManager::CommonVRAMRegionTaskSet::Reset()
{
    numTasksPerPriority_[0] = 0;
    numTasksPerPriority_[1] = 0;
    numTasks_ = 0;
}

void SendStagedVRAMDataToVRAM(void* vramStagingManagerUserdata)
{
    if (vramStagingManagerUserdata == NULL)
        return;
    ((VRAMStagingManager*)vramStagingManagerUserdata)->SendReadyDataToVRAM();
}