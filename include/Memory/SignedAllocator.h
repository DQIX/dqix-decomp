#pragma once

#include <globaldefs.h>

// There seems to be a tree-like structure handling all memory allocation.
// There are multiple types of allocator, and the type of an allocator is 
// indicated by the first four bytes of it in memory (a signature).
// Every allocator has its own contiguous chunk of memory which it can allocate
// to the user in its own way. Within an allocator you may allocate a series
// of new allocators, each using a subspace of the parent allocator's space.
// This forms a tree, where the allocators within a large allocator are considered
// children. Each allocator holds pointers to the previous and next child of
// its parent (its immediate siblings, so to speak) as well as a linked list
// for its own children.

#define ALLOCATOR_SIGNATURE_HPXE 0x45585048
#define ALLOCATOR_SIGNATURE_HMRF 0x46524d48
#define ALLOCATOR_SIGNATURE_HTNU 0x554e5448

struct SignedAllocatorHeader;

class SignedAllocatorList
{
public:
    SignedAllocatorHeader* pFirst;
    SignedAllocatorHeader* pLast;
    unsigned short numElements;
    unsigned short signatureLength; // always 4

    void Initialize(unsigned short signatureLength);
    void DoInitialInsertion(SignedAllocatorHeader* what);

    void InsertAtEnd(SignedAllocatorHeader* what);
    void InsertAtStart(SignedAllocatorHeader* what);
    void InsertBefore(SignedAllocatorHeader* where, SignedAllocatorHeader* what);
    void Remove(SignedAllocatorHeader* what);

    SignedAllocatorHeader* ElementAfter(SignedAllocatorHeader* what);
    SignedAllocatorHeader* ElementBefore(SignedAllocatorHeader* what);

    SignedAllocatorHeader* GetNthElement(unsigned int n);
};

struct SignedAllocatorHeader
{
    unsigned int signature;
    SignedAllocatorHeader* pPrevAllocator;
    SignedAllocatorHeader* pNextAllocator;
    SignedAllocatorList children;
    void* allocBegin;
    void* allocEnd;
    int clearFlags; // the type on this is a mess...
};

namespace AllocatorTree
{
    // Yes, this is an int and not a bool. It's handled like a bool though.
    // This makes me think this part might have been written in C, since it
    // already seems likely to be from an external library (and we know
    // parts/all of it use different compiler settings)
    extern int initializedTopLevel;

    // At the root of the tree we don't have an allocator, so we just store a 
    // global list of 'top-level' allocators. Can also view this as a forest.
    extern SignedAllocatorList topLevel;

    // This function works recursively, so you have to pass the top level
    // as its first parameter. The search takes place in the subtree rooted
    // at the node whose children are given in the list you provide.
    // Returns NULL if the allocator can't be found.
    SignedAllocatorHeader* GetParent(SignedAllocatorList* searchTree, SignedAllocatorHeader* alloc);

    SignedAllocatorList* GetListContainingAllocator(SignedAllocatorHeader* allocator);

    void InsertNewAndPopulateHeader(SignedAllocatorHeader* allocator, unsigned int sig,
        void* allocBegin, void* allocEnd, unsigned short clearFlags);
    void Remove(SignedAllocatorHeader* alloc);

}