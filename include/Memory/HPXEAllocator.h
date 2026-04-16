#pragma once

#include <globaldefs.h>

// I don't know what HPXE stands for, but the class features a magic constant
// which looks like this in memory, so that's what I'm calling it. There is a 
// decent chance that it's actually EXPH but this implementation assumes a big
// endian system (see comment about block signature later).
// Functionally this is a doubly linked list, with options to either search the list
// from front-to-back or vice versa for a suitable block.
class HPXEAllocator
{
public:
    struct Block
    {
        // Either 0x4652 for a free block or 0x5544 for a used block.
        // In memory these look like RF and DU. Maybe they were intended
        // to be the other way around (FRee and UseD) but endianness messed
        // things up?
        unsigned short signature;
        // This is really storing three things in one (I would have liked to 
        // use bitfields, but the assembly didn't work out).
        // Bits 0-7: copy of BlockManager::unknown_34
        // Bits 8-14: number of wasted bytes preceding this
        // Bit 15: 1 if allocated from end, 0 if from front.
        // These would be useful things for coalescing multiple fragmented
        // blocks, though I haven't found the code doing this yet. 
        unsigned short coalescenceData;
        // This is the extra size, not including the 16 bytes of header
        // used by these variables.
        unsigned int size;
        Block* pPrev;
        Block* pNext;
    };

    struct BlockList
    {
        Block* pFirst;
        Block* pLast;

        Block* RemoveAndGetPrevious(Block* block);
        // Returns the inserted block.
        Block* InsertAfter(Block* newBlock, Block* previous);
    };

    // Describes a (separately held) contiguous range of memory, a priori
    // not used for any particular purpose. (In practice, blocks will end
    // up being created within the range described).
    struct MemoryRangeDescription
    {
        // unsigned int instead of void* bc we do a lot of calculations with them
        unsigned int beginAddr;
        unsigned int endAddr;

        void PopulateFromBlock(Block* block);
        Block* CreateBlock(unsigned short signature);
    };

    struct BlockManager
    {
        BlockList freeBlocks;
        BlockList usedBlocks;
        unsigned short unknown_34;
        // if lowest bit is set, we take the smallest possible block, otherwise
        // just take the first acceptable block.
        unsigned short blockSelectFlags;

        void* AllocateFromFreeBlock(Block* freeNode, unsigned int effStart, unsigned int length, unsigned short reversed);
        bool FreeMemoryRange(const MemoryRangeDescription& range);
    };
    
public:
    unsigned int signature; // always 0x45585048 ("HXPE")
    char unknown_0[0x14];
    // We have a linked list of blocks but all of them are carved out of
    // one contiguous region
    void* allocBegin;
    void* allocEnd;
    unsigned char clearFlags;
    BlockManager blocks;

private:
    // Creates an allocator at location start with a single block of available
    // data running up to end. (This block will gradually be fragmented as
    // allocations occur).
    // If the bottom bit of clearFlags is set, then all memory will be
    // zeroed out whenever allocated.
    static HPXEAllocator* CreateInRegion(void* start, void* end, unsigned short clearFlags);

    void* AllocateFromFront(unsigned int len, unsigned int align);
    void* AllocateFromEnd(unsigned int len, unsigned int align);

public:
    static HPXEAllocator* CreateAtLocation(void* where, unsigned int size, unsigned short clearFlags);

    void* Allocate(unsigned int len, int alignAndDir);
    void Free(void* data);

    unsigned int GetMaxPossibleAllocation(int alignAndDir);
};