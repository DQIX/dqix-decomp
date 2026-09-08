#include "Memory/SafeAllocator.h"
#include "Resource/ResourceMutex.h"

int safeAllocatorShouldIncCount = 1;
unsigned int safeAllocatorLiveCount = 0;

// USA: func_0203246c
// JPN: func_02031fa4
unsigned int SafeAllocator::GetLiveCount()
{
    return safeAllocatorLiveCount;
}

// USA: func_0203247c
// JPN: func_02031fb4
void SafeAllocator::ResetAllocatorPointer()
{
    pSignedAlloc = NULL;
}

// USA: func_02032488
// JPN: func_02031fc0
void SafeAllocator::CreateTypeB(void* bufferStart, unsigned int bufferSize, int alignAndDir)
{
    LockResourceMutex();

    // 0 as third parameter means memory isn't cleared on allocation
    pSignedAlloc = &HPXEAllocator::CreateAtLocation(bufferStart, bufferSize, 0)->header;
    allocUnion.InitializeTypeB((HPXEAllocator*)pSignedAlloc, alignAndDir);

    if (safeAllocatorShouldIncCount)
        safeAllocatorLiveCount++;

    UnlockResourceMutex();
}

// USA: func_020324f0
// JPN: func_02032028
void SafeAllocator::CreateTypeA(void* bufferStart, unsigned int bufferSize)
{
    LockResourceMutex();

    // 0 as third parameter means memory isn't cleared on allocation
    pSignedAlloc = &HMRFAllocator::CreateAtLocation(bufferStart, bufferSize, 0)->header;
    allocUnion.InitializeTypeA((HMRFAllocator*)pSignedAlloc, 4);
    safeAllocatorLiveCount++;

    UnlockResourceMutex();
}

// USA: func_02032544
// JPN: func_0203207c
void* SafeAllocator::Allocate(unsigned int len)
{
    if (pSignedAlloc == NULL)
        return NULL;

    LockResourceMutex();
    void* ret = allocUnion.Allocate((len + 3) & ~3);
    UnlockResourceMutex();

    return ret;
}

// USA: func_02032584
// JPN: func_020320bc
void* SafeAllocator::AllocateReversed(unsigned int len)
{
    if (pSignedAlloc == NULL)
        return NULL;

    LockResourceMutex();

    void* ret = NULL;

    switch (pSignedAlloc->signature)
    {
    case ALLOCATOR_SIGNATURE_HPXE:
        ret = allocUnion.versions.typeB.pHPXEAllocator->Allocate(len, -4);
        break;
    case ALLOCATOR_SIGNATURE_HMRF:
        ret = allocUnion.versions.typeA.pHMRFAllocator->Allocate(len, -4);
        break;
    case ALLOCATOR_SIGNATURE_HTNU: // not elided!
        break;
    }

    UnlockResourceMutex();
    return ret;
}

// USA: func_02032618
// JPN: func_02032150
void SafeAllocator::Free(void* data)
{
    if (pSignedAlloc == NULL)
        return;

    LockResourceMutex();

    switch (pSignedAlloc->signature)
    {
    case ALLOCATOR_SIGNATURE_HPXE:
        ((HPXEAllocator*)pSignedAlloc)->Free(data);
        break;
    case ALLOCATOR_SIGNATURE_HMRF:
        ((HMRFAllocator*)pSignedAlloc)->Free(2 | 1);
        break;
    case ALLOCATOR_SIGNATURE_HTNU:
        break;
    }

    UnlockResourceMutex();
}

// USA: func_02032688
// JPN: func_020321c0
void SafeAllocator::Reset()
{
    if (pSignedAlloc == NULL)
        return;
    
    LockResourceMutex();

    void* allocStart = pSignedAlloc;
    unsigned int allocSize = GetSize();
    switch (pSignedAlloc->signature)
    {
    case ALLOCATOR_SIGNATURE_HPXE:
        ((HPXEAllocator*)pSignedAlloc)->RemoveFromTree();
        safeAllocatorShouldIncCount = 0;
        CreateTypeB(allocStart, allocSize, 4); // this reinserts into the tree
        safeAllocatorShouldIncCount = 1;
        break;
    case ALLOCATOR_SIGNATURE_HMRF:
        ((HMRFAllocator*)pSignedAlloc)->Free(2 | 1);
        break;
    case ALLOCATOR_SIGNATURE_HTNU:
        break;
    }

    UnlockResourceMutex();
}

// USA: func_02032730
// JPN: func_02032268
void SafeAllocator::Destroy()
{
    if (pSignedAlloc == NULL)
        return;

    LockResourceMutex();

    bool removed = false;
    switch (pSignedAlloc->signature)
    {
    case ALLOCATOR_SIGNATURE_HPXE:
        ((HPXEAllocator*)pSignedAlloc)->RemoveFromTree();
        removed = true;
        break;
    case ALLOCATOR_SIGNATURE_HMRF:
        ((HMRFAllocator*)pSignedAlloc)->RemoveFromTree();
        removed = true;
        break;
    case ALLOCATOR_SIGNATURE_HTNU:
        break;
    }

    if (removed)
        safeAllocatorLiveCount--;

    pSignedAlloc = NULL;

    UnlockResourceMutex();
}

// USA: func_020327c0
// JPN: func_020322f8
unsigned int SafeAllocator::GetSize() const
{
    if (pSignedAlloc == NULL)
        return 0;

    LockResourceMutex();
    unsigned int len = (unsigned int)pSignedAlloc->allocEnd - (unsigned int)pSignedAlloc;
    UnlockResourceMutex();
    return len;
}

// USA: func_020327f4
// JPN: func_0203232c
unsigned int SafeAllocator::GetMaxPossibleAllocation() const
{
    if (pSignedAlloc == NULL)
        return 0;

    LockResourceMutex();

    unsigned int ret = 0;
    switch (pSignedAlloc->signature)
    {
    case ALLOCATOR_SIGNATURE_HPXE:
        ret = ((HPXEAllocator*)pSignedAlloc)->GetMaxPossibleAllocation(4);
        break;
    case ALLOCATOR_SIGNATURE_HMRF:
        ret = ((HMRFAllocator*)pSignedAlloc)->GetMaxPossibleAllocation(4);
        break;
    case ALLOCATOR_SIGNATURE_HTNU:
        break;
    }

    UnlockResourceMutex();
    return ret;
}

// USA: func_02032874
// JPN: func_020323ac
unsigned int SafeAllocator::GetSizeWithLargestBlockRemoved() const
{
    if (pSignedAlloc == NULL)
        return 0;

    LockResourceMutex();
    unsigned int ret = GetSize() - GetMaxPossibleAllocation();
    UnlockResourceMutex();
    return ret;
}

// USA: func_020328b4
// JPN: func_020323ec
SignedAllocatorHeader* SafeAllocator::GetSignedAllocator() const
{
    return pSignedAlloc;
}