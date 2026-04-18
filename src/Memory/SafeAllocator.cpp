#include "Memory/SafeAllocator.h"

extern "C"
{
    void func_020d970c();
    void func_020d974c();
}

extern int safeAllocatorShouldIncCount;
extern unsigned int safeAllocatorLiveCount;

// USA: func_0203246c
unsigned int SafeAllocator::GetLiveCount()
{
    return safeAllocatorLiveCount;
}

// USA: func_0203247c
void SafeAllocator::ResetAllocatorPointer()
{
    pSignedAlloc = NULL;
}

// USA: func_02032488
void SafeAllocator::CreateTypeB(void* bufferStart, unsigned int bufferSize, int alignAndDir)
{
    func_020d970c();

    // 0 as third parameter means memory isn't cleared on allocation
    pSignedAlloc = &HPXEAllocator::CreateAtLocation(bufferStart, bufferSize, 0)->header;
    allocUnion.InitializeTypeB((HPXEAllocator*)pSignedAlloc, alignAndDir);

    if (safeAllocatorShouldIncCount)
        safeAllocatorLiveCount++;

    func_020d974c();
}

// USA: func_020324f0
void SafeAllocator::CreateTypeA(void* bufferStart, unsigned int bufferSize)
{
    func_020d970c();

    // 0 as third parameter means memory isn't cleared on allocation
    pSignedAlloc = &HMRFAllocator::CreateAtLocation(bufferStart, bufferSize, 0)->header;
    allocUnion.InitializeTypeA((HMRFAllocator*)pSignedAlloc, 4);
    safeAllocatorLiveCount++;

    func_020d974c();
}

// USA: func_02032544
void* SafeAllocator::Allocate(unsigned int len)
{
    if (pSignedAlloc == NULL)
        return NULL;

    func_020d970c();
    void* ret = allocUnion.Allocate((len + 3) & ~3);
    func_020d974c();

    return ret;
}

// USA: func_02032584
void* SafeAllocator::AllocateReversed(unsigned int len)
{
    if (pSignedAlloc == NULL)
        return NULL;

    func_020d970c();

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

    func_020d974c();
    return ret;
}

// USA: func_02032618
void SafeAllocator::Free(void* data)
{
    if (pSignedAlloc == NULL)
        return;

    func_020d970c();

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

    func_020d974c();
}

// USA: func_02032688
void SafeAllocator::Reset()
{
    if (pSignedAlloc == NULL)
        return;
    
    func_020d970c();

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

    func_020d974c();
}

// USA: func_02032730
void SafeAllocator::Destroy()
{
    if (pSignedAlloc == NULL)
        return;

    func_020d970c();

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

    func_020d974c();
}

// USA: func_020327c0
unsigned int SafeAllocator::GetSize() const
{
    if (pSignedAlloc == NULL)
        return 0;

    func_020d970c();
    unsigned int len = (unsigned int)pSignedAlloc->allocEnd - (unsigned int)pSignedAlloc;
    func_020d974c();
    return len;
}

// USA: func_020327f4
unsigned int SafeAllocator::GetMaxPossibleAllocation() const
{
    if (pSignedAlloc == NULL)
        return 0;

    func_020d970c();

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

    func_020d974c();
    return ret;
}

// USA: func_02032874
unsigned int SafeAllocator::GetSizeWithLargestBlockRemoved() const
{
    if (pSignedAlloc == NULL)
        return 0;

    func_020d970c();
    unsigned int ret = GetSize() - GetMaxPossibleAllocation();
    func_020d974c();
    return ret;
}

// USA: func_020328b4
SignedAllocatorHeader* SafeAllocator::GetSignedAllocator() const
{
    return pSignedAlloc;
}