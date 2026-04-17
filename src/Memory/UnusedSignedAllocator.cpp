#include "Memory/UnusedSignedAllocator.h"

extern "C"
{
    // Yet another memset but with stupid parameter order and assumes
    // 4-byte alignment (it writes ints).
    // Notably, if dst is not 4-byte aligned it might write before the start,
    // and if len is not a multiple of 4 it might write a bit past the end.
    void func_020ca3ec(int value, void* start, unsigned length);
}

UnusedSignedAllocator::Block* UnusedSignedAllocator::PopFrontAndUpdate(Block** ppBlock)
{
    Block* oldFront = *ppBlock;
    if (oldFront != NULL)
    {
        *ppBlock = oldFront->pNext;
    }
    return oldFront;
}

void* UnusedSignedAllocator::Allocate()
{
    Block* ret = PopFrontAndUpdate(&pFirstAvailableBlock);
    
    if (ret != NULL)
    {
        unsigned int clearLength = blockSize;
        if ((unsigned char)header.clearFlags & 1)
            func_020ca3ec(0, ret, clearLength);
    }
    return ret;
}

void UnusedSignedAllocator::Free(void* data)
{
    Block* newBlock = (Block*)data;
    newBlock->pNext = pFirstAvailableBlock;
    pFirstAvailableBlock = newBlock;
}