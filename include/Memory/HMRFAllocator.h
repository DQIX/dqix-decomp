#pragma once

#include "SignedAllocator.h"

// A class managing a block of memory from which we can allocate
// either from the start going forwards or from the end going backwards.
class HMRFAllocator
{
public:
    // The allocator stores a single-linked list of previous states, stored
    // from newest to oldest. These are allocated within the block. You can
    // save the current state with an id and restore it later, giving the
    // functionality of a stack frame or similar.
    struct SavedState
    {
        int id;
        unsigned int startAddress;
        unsigned int endAddress;
        SavedState* pPrev;
    };

    struct Block
    {
        unsigned int startAddress;
        unsigned int endAddress; // points past the end, like vector::end() etc

        // For proper behaviour alignment is expected to be a power of 2
        void* AllocateForward(unsigned int len, unsigned int alignment);
        void* AllocateBackward(unsigned int len, unsigned int alignment);
    };
public:
    SignedAllocatorHeader header;
    Block block;
    SavedState* newestState;

    // If the bottom bit of clearFlags is set, then all memory will be
    // zeroed out whenever allocated.
    static HMRFAllocator* CreateInRegion(void* start, void* end, unsigned short clearFlags);

    void FreeFront();
    void FreeBack();

    static HMRFAllocator* CreateAtLocation(void* where, unsigned int size, unsigned short clearFlags);
    void RemoveFromTree();

    void* Allocate(unsigned int len, int alignAndDirection);
    // If bit 0 is set, frees everything at the front.
    // If bit 1 is set, frees everything at the back.
    // Note this breaks saved states: if the back is freed, restoring an earlier
    // state won't recover those bits, and if the front is freed no states can
    // be restored at all.
    void Free(int flags);

    unsigned int GetMaxPossibleAllocation(int alignAndDir);

    bool SaveCurrentState(int id);
    bool RestoreState(int id);
};