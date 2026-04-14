#include "Memory/DoubleEndedArenaAllocator.h"

// USA: func_020af9b8
void* DoubleEndedArenaAllocator::Allocate(unsigned int len, int alignAndDir)
{
    if (len == 0)
        len = 1;
    
    unsigned int fourByteAlignedLength = (len + 3) & ~3;
    if (alignAndDir >= 0)
        return block.range.AllocateForward(fourByteAlignedLength, alignAndDir);
    else
        return block.range.AllocateBackward(fourByteAlignedLength, -alignAndDir);
}