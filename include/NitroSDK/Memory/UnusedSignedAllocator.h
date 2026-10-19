#pragma once

#include "SignedAllocator.h"

// In addition to not being used, only referenced in an unused vtable,
// this class seems to have a much more limited interface than the other two.
// Perhaps it had other functions but the compiler/linker elided them because
// it saw that they never got called?
// 
// There are various functions that take a SignedAllocatorHeader and call
// a member function of HPXE/HMRFAllocator based on the signature, e.g.
// func_02032584. These have an unused signature 0x554e5448
// ("HTNU" or maybe "UNTH") in the binary. It seems likely that this class
// corresponds to it, but can't be sure.
//
// In any case, this allocator is the simplest of the three: it just has a
// linked list of blocks, all the same size, and allocating is a case of taking
// the front block out and repurposing it. The block order is not important,
// and there is no need to perform coalescence with freed blocks, meaning there
// is no need for any header data within a block (except the pointer to next,
// which can be overwritten on allocation).
class UnusedSignedAllocator
{
public:
    struct Block
    {
        Block* pNext;
        // ...extra bytes...
    };

    SignedAllocatorHeader header;
    Block* pFirstAvailableBlock; // just first in the linked list
    unsigned int blockSize;

private:
    // To be called with &pFirstAvailableBlock to update it.
    // Most likely pFirstAvailableBlock is within a struct/class of its own
    // and the function parameter is actually the this pointer, but that's more
    // fuss than it's worth for a class this simple (and never used)
    static Block* PopFrontAndUpdate(Block** ppBlock);

public:
    // Remove the front block, thus allocating it for the caller
    void* Allocate();
    // Add a new block, making it available for future callers (i.e. freeing)
    void Free(void* data);
};