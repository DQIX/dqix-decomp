#include "Memory/FixedSizeBlockList.h"

extern "C"
{
    // Yet another memset implementation
    void func_020ca3ec(int value, void* start, unsigned length);
}

FixedSizeBlockList::Block* PopAndUpdateList(FixedSizeBlockList::Block** ppBlock)
{
    FixedSizeBlockList::Block* oldFront = *ppBlock;
    if (oldFront)
    {
        *ppBlock = oldFront->pNext;
    }
    return oldFront;
}

void* FixedSizeBlockList::RemoveFrontBlock()
{
    Block* ret = PopAndUpdateList(&pFirstAvailableBlock);
    
    if (ret != NULL)
    {
        unsigned int clearLength = blockSize;
        if ((unsigned char)allocFlags & 1)
            func_020ca3ec(0, ret, clearLength);
    }
    return ret;
}

void FixedSizeBlockList::InsertFrontBlock(void* data)
{
    Block* newBlock = (Block*)data;
    newBlock->pNext = pFirstAvailableBlock;
    pFirstAvailableBlock = newBlock;
}