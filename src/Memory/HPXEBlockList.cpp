#include "Memory/HPXEAllocator.h"

// This should probably get merged in with HPXEAllocator.cpp, it's just a
// separate file so I can decomp the functions in a saner order

#define BLOCKSIGNATURE_FREE 0x4652 // looks like "RF" in memory
#define BLOCKSIGNATURE_USED 0x5544 // looks like "DU" in memory
#define SIGNATURE_HPXE 0x45585048 // looks like "HXPE" in memory

// Sets the optimization level to O2,p instead of O2. It performs a small
// optimization in FreeMemoryRange().
#pragma optimize_for_size off

extern "C"
{
    HPXEAllocator* func_020af178(HPXEAllocator* alloc, unsigned int sig, void* start, void* end, unsigned short flags);

    void func_020ca3ec(int val, void* dst, unsigned int len);
}

// USA: func_020af20c
void HPXEAllocator::MemoryRangeDescription::PopulateFromBlock(Block* block)
{
    unsigned int dataStart = (unsigned int)block + sizeof(Block);
    unsigned short extraBytesBefore = (block->coalescenceData >> 8) & 0x7f;
    beginAddr = (unsigned int)block - extraBytesBefore;
    endAddr = block->size + dataStart;
}

// USA: func_020af238
HPXEAllocator::Block* HPXEAllocator::BlockList::RemoveAndGetPrevious(Block* block)
{
    Block* oldPrev = block->pPrev;
    Block* oldNext = block->pNext;

    if (oldPrev != NULL) 
        oldPrev->pNext = oldNext;
    else
        this->pFirst = oldNext;

    if (oldNext != NULL)
        oldNext->pPrev = oldPrev;
    else
        this->pLast = oldPrev;

    return oldPrev;
}

// USA: func_020af260
HPXEAllocator::Block* HPXEAllocator::BlockList::InsertAfter(Block* newBlock, Block* prev)
{
    Block* next;
    newBlock->pPrev = prev;
    if (prev != NULL)
    {
        next = prev->pNext;
        prev->pNext = newBlock;
    }
    else
    {
        next = this->pFirst;
        this->pFirst = newBlock;
    }

    newBlock->pNext = next;
    if (next != NULL)
        next->pPrev = newBlock;
    else
        this->pLast = newBlock;

    return newBlock;
}

// USA: func_020af290
HPXEAllocator::Block* HPXEAllocator::MemoryRangeDescription::CreateBlock(unsigned short sig)
{
    Block* block = (Block*)beginAddr;
    block->signature = sig;
    block->coalescenceData = 0;
    unsigned int dataStart = (unsigned int)block + sizeof(Block);
    block->size = endAddr - dataStart;
    block->pPrev = NULL;
    block->pNext = NULL;
    return block;
}

// USA: func_020af2bc
HPXEAllocator* HPXEAllocator::CreateInRegion(void* start, void* end, unsigned short clearFlags)
{
    HPXEAllocator* alloc = (HPXEAllocator*)start;
    func_020af178(alloc, SIGNATURE_HPXE, (char*)start + sizeof(HPXEAllocator), end, clearFlags);
    alloc->blocks.unknown_34 = 0;
    alloc->blocks.blockSelectFlags = 0;
    
    // no preference for smallest possible block
    alloc->blocks.blockSelectFlags &= ~1;

    MemoryRangeDescription range;
    range.beginAddr = (unsigned int)alloc->allocBegin;
    range.endAddr = (unsigned int)alloc->allocEnd;
    Block* block = range.CreateBlock(BLOCKSIGNATURE_FREE);
    alloc->blocks.freeBlocks.pFirst = block;
    alloc->blocks.freeBlocks.pLast = block;
    alloc->blocks.usedBlocks.pFirst = 0;
    alloc->blocks.usedBlocks.pLast = 0;
    return alloc;
}

// USA: func_020af330
void* HPXEAllocator::BlockManager::AllocateFromFreeBlock(
    Block* initNode, unsigned int effStart, unsigned int length, unsigned short reversed)
{
    MemoryRangeDescription pieceBefore;
    MemoryRangeDescription pieceAfter;
    pieceBefore.PopulateFromBlock(initNode);
    pieceAfter.endAddr = pieceBefore.endAddr;
    pieceAfter.beginAddr = length + effStart;

    Block* prevFreeBlock;
    unsigned int trueStart = effStart - 0x10;
    pieceBefore.endAddr = trueStart;

    prevFreeBlock = freeBlocks.RemoveAndGetPrevious(initNode);
    if (pieceBefore.endAddr - pieceBefore.beginAddr < 0x14)
        pieceBefore.endAddr = pieceBefore.beginAddr;
    else
    {
        Block* tmp = pieceBefore.CreateBlock(BLOCKSIGNATURE_FREE);
        prevFreeBlock = freeBlocks.InsertAfter(tmp, prevFreeBlock);
    }

    if (pieceAfter.endAddr - pieceAfter.beginAddr < 0x14)
        pieceAfter.beginAddr = pieceAfter.endAddr;
    else
    {
        Block* tmp = pieceAfter.CreateBlock(BLOCKSIGNATURE_FREE);
        freeBlocks.InsertAfter(tmp, prevFreeBlock);
    }

    unsigned char clearFlags = *(unsigned int*)((char*)this - 4);
    unsigned int clearLength = pieceAfter.beginAddr - pieceBefore.endAddr;
    
    if (clearFlags & 1)
        func_020ca3ec(0, (void*)pieceBefore.endAddr, clearLength);
    
    MemoryRangeDescription thisPiece;
    thisPiece.beginAddr = trueStart;
    thisPiece.endAddr = pieceAfter.beginAddr;
    Block* usedSubblock = thisPiece.CreateBlock(BLOCKSIGNATURE_USED);

    
    usedSubblock->coalescenceData &= ~0x8000;
    unsigned int reversalBitmask = (unsigned int)reversed << 31;
    usedSubblock->coalescenceData |= reversalBitmask >> 16;


    unsigned short numBytesBefore = ((unsigned int)usedSubblock - pieceBefore.endAddr);
    usedSubblock->coalescenceData &= ~0x7f00;
    usedSubblock->coalescenceData |= (numBytesBefore & 0x7f) << 8;


    unsigned char lowByte = this->unknown_34;
    usedSubblock->coalescenceData &= ~0xff;
    usedSubblock->coalescenceData |= lowByte;
    

    usedBlocks.InsertAfter(usedSubblock, usedBlocks.pLast);
    return (void*)effStart;
}

// USA: func_020af4b0
void* HPXEAllocator::AllocateFromFront(unsigned int len, unsigned int align)
{
    bool acceptAnyBlock = (unsigned short)(blocks.blockSelectFlags & 1) == 0;
    
    Block* loopBlock = blocks.freeBlocks.pFirst;
    Block* bestBlock = NULL;
    
    unsigned int smallestBlockSize = 0xffffffff;
    unsigned int addrWithinBestBlock = 0;
    
    if (loopBlock != NULL)
    {
        unsigned int bitmask = ~(align - 1);
        do
        {
            unsigned int minStartAddr = (unsigned int)loopBlock + 0x10;
            // Want to start as close to beginning of block as possible, but have
            // to respect header size (0x10) and alignment
            unsigned int desiredStartAddr = (align - 1 + minStartAddr) & bitmask;
            unsigned int wastedBytes = desiredStartAddr - minStartAddr;

            if (loopBlock->size >= len + wastedBytes && smallestBlockSize > loopBlock->size)
            {
                bestBlock = loopBlock;
                smallestBlockSize = loopBlock->size;
                addrWithinBestBlock = desiredStartAddr;
                
                if (acceptAnyBlock || loopBlock->size == len)
                    break;
            }
            loopBlock = loopBlock->pNext;
        } while (loopBlock != NULL);
    }

    if (bestBlock == NULL)
        return NULL;

    return blocks.AllocateFromFreeBlock(bestBlock, addrWithinBestBlock, len, 0);
}

void* HPXEAllocator::AllocateFromEnd(unsigned int len, unsigned int align)
{
    bool acceptAnyBlock = (unsigned short)(blocks.blockSelectFlags & 1) == 0;
    
    Block* loopBlock = blocks.freeBlocks.pLast;
    Block* bestBlock = NULL;
    
    unsigned int smallestBlockSize = 0xffffffff;
    unsigned int addrWithinBestBlock = 0;
    
    if (loopBlock != NULL)
    {
        unsigned int bitmask = ~(align - 1);
        do
        {
            unsigned int minStartAddr = (unsigned int)loopBlock + 0x10;
            // go as close to end of block as possible, respecting alignment
            unsigned int desiredStartAddr = (loopBlock->size + minStartAddr - len) & bitmask;
            int remainingBytes = desiredStartAddr - minStartAddr;

            if (remainingBytes >= 0 && smallestBlockSize > loopBlock->size)
            {
                bestBlock = loopBlock;
                smallestBlockSize = loopBlock->size;
                addrWithinBestBlock = desiredStartAddr;
                
                if (acceptAnyBlock || loopBlock->size == len)
                    break;
            }
            loopBlock = loopBlock->pPrev;
        } while (loopBlock != NULL);
    }

    if (bestBlock == NULL)
        return NULL;

    return blocks.AllocateFromFreeBlock(bestBlock, addrWithinBestBlock, len, 1);
}

// https://decomp.me/scratch/BJC0E
// USA: func_020af614
bool HPXEAllocator::BlockManager::FreeMemoryRange(const MemoryRangeDescription& range)
{
    MemoryRangeDescription updatedRange = range;
    Block* prevBlock = NULL;

    if (freeBlocks.pFirst != NULL)
    {
        //Block* u1 = freeBlocks.pFirst;
        unsigned int minAddress = range.beginAddr;
        Block* loopBlock = freeBlocks.pFirst;
        do {
            if ((unsigned int)loopBlock < minAddress)
            {
                prevBlock = loopBlock;
            }
            else
            {
                if ((unsigned int)loopBlock == range.endAddr)
                {
                    unsigned int dataStart = (unsigned int)loopBlock + 0x10;
                    updatedRange.endAddr = loopBlock->size + dataStart;
                    freeBlocks.RemoveAndGetPrevious(loopBlock);
                }
                break;
            }
            loopBlock = loopBlock->pNext;
        } while (loopBlock != NULL);
    }

    if (prevBlock != 0)
    {
        unsigned int prevBlockEnd = prevBlock->size + ((unsigned int)prevBlock + sizeof(Block));
        if (prevBlockEnd == range.beginAddr)
        {
            updatedRange.beginAddr = (unsigned int)prevBlock;
            prevBlock = freeBlocks.RemoveAndGetPrevious(prevBlock);
        }
    }

    unsigned int len = updatedRange.endAddr - updatedRange.beginAddr;
    
    if (len < sizeof(Block))
        return false;

    Block* freeBlock = updatedRange.CreateBlock(BLOCKSIGNATURE_FREE);
    freeBlocks.InsertAfter(freeBlock, prevBlock);
    return true;
}

HPXEAllocator* HPXEAllocator::CreateAtLocation(void* where, unsigned int size, unsigned short clearFlags)
{
    unsigned int end = (size + (unsigned int)where) & ~3;
    unsigned int begin = ((unsigned int)where + 3) & ~3;
    
    // 0x4c = sizeof(HPXEAllocator) + sizeof(Block) + 4
    // (any less and we have no useful space)
    if (begin > end || end - begin < 0x4c)
        return 0;
    
    return HPXEAllocator::CreateInRegion((void*)begin, (void*)end, clearFlags);
}