#include "Memory/DoubleEndedBlock.h"
#include <globaldefs.h>

extern "C"
{
    // Yet another memset but with stupid parameter order and assumes
    // 4-byte alignment (it writes ints).
    // Notably, if dst is not 4-byte aligned it might write before the start,
    // and if len is not a multiple of 4 it might write a bit past the end.
    void func_020ca3ec(int value, void* dst, unsigned int len);
}

void* DoubleEndedBlock::Range::AllocateForward(unsigned int len, unsigned int align)
{
    unsigned int pieceTrueStart = startAddress;
    unsigned int pieceEffStart = (align - 1 + pieceTrueStart) & ~(align - 1);
    unsigned int pieceEnd = len + pieceEffStart;

    if (pieceEnd > this->endAddress)
        return NULL;

    // Really, we're doing this
    DoubleEndedBlock* outer = (DoubleEndedBlock*)((char*)this - 4);
    unsigned int clearLength = pieceEnd - pieceTrueStart;
    // This looks potentially dangerous, trueStart is not required to be 4-byte
    // aligned, and if alignment checking is disabled the address will be
    // rounded down, potentially overwriting data from the previous allocation
    // https://developer.arm.com/documentation/dui0379/g/Using-the-Assembler/Address-alignment
    if ((unsigned char)outer->flags & 1)
        func_020ca3ec(0, (void*)pieceTrueStart, clearLength);
    
    this->startAddress = pieceEnd;
    return (void*)pieceEffStart;    
}

void* DoubleEndedBlock::Range::AllocateBackward(unsigned int len, unsigned int align)
{
    unsigned int pieceStart = (this->endAddress - len) & ~(align - 1);

    if (pieceStart < this->startAddress)
        return NULL;
    
    DoubleEndedBlock* outer = (DoubleEndedBlock*)((char*)this - 4);
    unsigned int clearLength = this->endAddress - pieceStart;
    // This one should be fine *provided* that the end address starts out being
    // 4-byte aligned and align >= 4 in all calls.
    if ((unsigned char)outer->flags & 1)
        func_020ca3ec(0, (void*)pieceStart, clearLength);
    
    this->endAddress = pieceStart;
    return (void*)pieceStart;
}