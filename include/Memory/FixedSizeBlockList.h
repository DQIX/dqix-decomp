#pragma once

#include <globaldefs.h>

// This seems to be used in the base version of a (virtual) allocator class.
// By the look of things, this version doesn't ever get used, but I'm putting
// it in anyway to allow setting up the override versions via C++ polymorphism
// instead of having to manually implement a vtable.
// This is a singly linked list whose nodes can be removed and repurposed as
// dynamic memory.
class FixedSizeBlockList
{
public:
    struct Block
    {
        Block* pNext;
        // ...extra bytes...
    };

    char unknown[0x20];
    int allocFlags; // if the LSB is set, allocated memory will be zeroed. Not sure what the rest does.
    Block* pFirstAvailableBlock; // just first in the linked list
    unsigned int blockSize;

    // Remove the front block, thus allocating it for the caller
    void* RemoveFrontBlock();
    // Add a new block, making it available for future callers (i.e. freeing)
    void InsertFrontBlock(void* data);
};