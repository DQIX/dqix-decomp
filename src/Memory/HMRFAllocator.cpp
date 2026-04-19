#include "Memory/HMRFAllocator.h"

#ifdef jpn
#define func_020017a4 func_020016c0
#define func_020ca3ec func_020cbeb8
#endif

extern "C"
{
    // Either abs or labs (they are identical as long == int on this system,
    // but there are two of them and both get used)
    int func_020017a4(int);

    // Yet another memset but with stupid parameter order and assumes
    // 4-byte alignment (it writes ints).
    // Notably, if dst is not 4-byte aligned it might write before the start,
    // and if len is not a multiple of 4 it might write a bit past the end.
    void func_020ca3ec(int value, void* dst, unsigned int len);
}

// USA: func_020af838
// JPN: func_020b1304
HMRFAllocator* HMRFAllocator::CreateInRegion(void* start, void* end, unsigned short clearFlags)
{
    HMRFAllocator* alloc = (HMRFAllocator*)start;
    AllocatorTree::InsertNewAndPopulateHeader(&alloc->header,
        ALLOCATOR_SIGNATURE_HMRF, (char*)start + sizeof(HMRFAllocator), end, clearFlags);
    alloc->block.startAddress = (unsigned int)alloc->header.allocBegin;
    alloc->block.endAddress = (unsigned int)alloc->header.allocEnd;
    alloc->newestState = NULL;

    return alloc;
}

// USA: func_020af880
// JPN: func_020b134c
void* HMRFAllocator::Block::AllocateForward(unsigned int len, unsigned int align)
{
    unsigned int pieceTrueStart = startAddress;
    unsigned int pieceEffStart = (align - 1 + pieceTrueStart) & ~(align - 1);
    unsigned int pieceEnd = len + pieceEffStart;

    if (pieceEnd > this->endAddress)
        return NULL;

    unsigned char clearFlags = *(int*)((char*)this - 4);
    unsigned int clearLength = pieceEnd - pieceTrueStart;
    // This looks potentially dangerous, trueStart is not required to be 4-byte
    // aligned, and if alignment checking is disabled the address will be
    // rounded down, potentially overwriting data from the previous allocation
    // https://developer.arm.com/documentation/dui0379/g/Using-the-Assembler/Address-alignment
    if (clearFlags & 1)
        func_020ca3ec(0, (void*)pieceTrueStart, clearLength);
    
    this->startAddress = pieceEnd;
    return (void*)pieceEffStart;    
}

// USA: func_020af8dc
// JPN: func_020b13a8
void* HMRFAllocator::Block::AllocateBackward(unsigned int len, unsigned int align)
{
    unsigned int pieceStart = (this->endAddress - len) & ~(align - 1);

    if (pieceStart < this->startAddress)
        return NULL;
    
    unsigned char clearFlags = *(int*)((char*)this - 4);
    unsigned int clearLength = this->endAddress - pieceStart;
    // This one should be fine *provided* that the end address starts out being
    // 4-byte aligned and align >= 4 in all calls.
    if (clearFlags & 1)
        func_020ca3ec(0, (void*)pieceStart, clearLength);
    
    this->endAddress = pieceStart;
    return (void*)pieceStart;
}

// USA: func_020af934
// JPN: func_020b1400
void HMRFAllocator::FreeFront()
{
    block.startAddress = (unsigned int)header.allocBegin;
    // resetting the start address deallocates all the states, so just need
    // to update the pointer.
    newestState = NULL;
}

// USA: func_020af948
// JPN: func_020b1414
void HMRFAllocator::FreeBack()
{
    SavedState* state = newestState;
    if (state != NULL)
    {
        do
        {
            state->endAddress = (unsigned int)header.allocEnd;
            state = state->pPrev;
        } while (state != NULL);
    }
    
    block.endAddress = (unsigned int)header.allocEnd;
}

// USA: func_020af974
// JPN: func_020b1440
HMRFAllocator* HMRFAllocator::CreateAtLocation(void* where, unsigned int size, unsigned short clearFlags)
{
    unsigned int end = (size + (unsigned int)where) & ~3;
    unsigned int begin = ((unsigned int)where + 3) & ~3;
    
    // Interestingly, unlike HPXE this lets you create an allocator with 
    // 0 bytes of general use space
    if (begin > end || end - begin < sizeof(HMRFAllocator))
        return NULL;
    
    return CreateInRegion((void*)begin, (void*)end, clearFlags);
}

// USA: func_020af9ac
// JPN: func_020b1478
void HMRFAllocator::RemoveFromTree()
{
    AllocatorTree::Remove(&header);
}

// USA: func_020af9b8
// JPN: func_020b1484
void* HMRFAllocator::Allocate(unsigned int len, int alignAndDir)
{
    if (len == 0)
        len = 1;
    
    unsigned int fourByteAlignedLength = (len + 3) & ~3;
    if (alignAndDir >= 0)
        return block.AllocateForward(fourByteAlignedLength, alignAndDir);
    else
        return block.AllocateBackward(fourByteAlignedLength, -alignAndDir);
}

// USA: func_020af9ec
// JPN: func_020b14b8
void HMRFAllocator::Free(int flags)
{
    if (flags & 1)
        FreeFront();

    if (flags & 2)
        FreeBack();
}

// USA: func_020afa18
// JPN: func_020b14e4
unsigned int HMRFAllocator::GetMaxPossibleAllocation(int alignAndDir)
{
    unsigned int align = func_020017a4(alignAndDir);

    unsigned int maxEnd = block.endAddress;

    unsigned int alignRounding = align - 1;
    unsigned int bitmask = ~(align - 1);

    unsigned int minStart = (align - 1 + block.startAddress) & bitmask;
    if (minStart > maxEnd)
        return 0;

    return maxEnd - minStart;
}

// USA: func_020afa50
// JPN: func_020b151c
bool HMRFAllocator::SaveCurrentState(int id)
{
    unsigned int currentFront = block.startAddress;
    SavedState* state = (SavedState*)block.AllocateForward(sizeof(SavedState), 4);
    if (state == NULL)
        return false;
       
    state->id = id;
    state->startAddress = currentFront;
    state->endAddress = block.endAddress;
    state->pPrev = newestState;

    newestState = state;
    return true;
}

// USA: func_020afaa0
// JPN: func_020b156c
bool HMRFAllocator::RestoreState(int id)
{
    SavedState* loopState = newestState;
    if (id != 0 && loopState != NULL)
    {
        do
        {
            if (loopState->id == id)
                break;
            else
                loopState = loopState->pPrev;
        } while (loopState != NULL);
    }

    if (loopState != NULL)
    {
        block.startAddress = loopState->startAddress;
        block.endAddress = loopState->endAddress;
        newestState = loopState->pPrev;
        return true;
    }

    return false;
}