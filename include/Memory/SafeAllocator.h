#pragma once

#include "AllocatorUnion.h"

// This appears to be the most abstracted form of a memory allocator,
// and also the one used in the most high-level code. It seems to be a wrapper
// for AllocatorUnion / the signed allocator types, with some auxiliary
// function calls (USA: func_020d970c and func_020d974c) before and after it.
// These mess with the interrupt state and increment/decrement a variable
// at a fixed location - maybe a mutex lock/unlock? Needs more investigation.
class SafeAllocator
{
public:
    SignedAllocatorHeader* pSignedAlloc;
    AllocatorUnion allocUnion;

    // total number of currently existent SafeAllocator instances
    static unsigned int GetLiveCount();

    void ResetAllocatorPointer();

    void CreateTypeB(void* bufferStart, unsigned int bufferSize, int alignAndDir);
    void CreateTypeA(void* bufferStart, unsigned int bufferSize);

    void* Allocate(unsigned int len);
    // allocate from the end of the block / last node in the free list
    void* AllocateReversed(unsigned int len);

    // if this is type A, it'll free everything
    void Free(void* data);

    // Clear all allocations and start fresh
    void Reset();

    void Destroy();

    // Returns the size of the externally stored part. Includes any
    // header/metadata used in that part.
    unsigned int GetSize() const;

    unsigned int GetMaxPossibleAllocation() const;

    // Equivalent to GetSize() - GetMaxPossibleAllocation().
    // For some reason every call to this is just discarding the return value,
    // which seems very pointless... maybe it was used for debugging, and
    // the release version didn't elide the call?
    unsigned int GetSizeWithLargestBlockRemoved() const;

    SignedAllocatorHeader* GetSignedAllocator() const;
};