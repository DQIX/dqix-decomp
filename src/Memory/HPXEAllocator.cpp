#include "Memory/HPXEAllocator.h"

extern "C"
{
    // Either abs or labs (they are identical as long == int on this system,
    // but there are two of them and both get used)
    int func_020017a4(int);
}

// USA: func_020af748
void* HPXEAllocator::Allocate(unsigned int len, int alignAndDir)
{
    if (len == 0)
        len = 1;
    
    unsigned int fourByteAlignedLength = (len + 3) & ~3;
    if (alignAndDir >= 0)
        return AllocateFromFront(fourByteAlignedLength, alignAndDir);
    else
        return AllocateFromEnd(fourByteAlignedLength, -alignAndDir);
}

// USA: func_020af778
void HPXEAllocator::Free(void* data)
{
    Block* pBlock = (Block*)((char*)data - sizeof(Block));
    MemoryRangeDescription range;

    range.PopulateFromBlock(pBlock);
    blocks.usedBlocks.RemoveAndGetPrevious(pBlock);
    blocks.FreeMemoryRange(range);
}

// USA: func_020af7b4
unsigned int HPXEAllocator::GetMaxPossibleAllocation(int alignAndDir)
{
    unsigned int align = func_020017a4(alignAndDir);

    unsigned int maxUsableSpace = 0;
    unsigned int minWastedSpace = 0xFFFFFFFF;
    
    Block* loopBlock = blocks.freeBlocks.pFirst;
    
    if (loopBlock != NULL)
    {    
        unsigned int alignRounding = align - 1;
        unsigned int bitmask = ~(align - 1);
        do
        {
            unsigned int blockHeaderEnd = (unsigned int)loopBlock + sizeof(Block);
            unsigned int blockUsableStart = (alignRounding + blockHeaderEnd) & bitmask;
            unsigned int blockUsableEnd = loopBlock->size + blockHeaderEnd;

            if (blockUsableStart < blockUsableEnd)
            {
                unsigned int thisUsableSpace = blockUsableEnd - blockUsableStart;
                unsigned int thisWastedSpace = blockUsableStart - blockHeaderEnd;
                if (maxUsableSpace < thisUsableSpace ||
                   (maxUsableSpace == thisUsableSpace && minWastedSpace > thisWastedSpace))
                {
                    maxUsableSpace = thisUsableSpace;
                    minWastedSpace = thisWastedSpace;
                }
            }
            
            loopBlock = loopBlock->pNext;
        } while (loopBlock != NULL);
    }
    
    return maxUsableSpace;
}