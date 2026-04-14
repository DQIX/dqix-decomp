#pragma once

// A class managing a block of memory from which we can allocate
// either from the start going forwards or from the end going backwards.
class DoubleEndedBlock
{
public:
    // bit 0: if set, memory is cleared on allocation.
    int flags;
    struct Range
    {
        unsigned int startAddress;
        unsigned int endAddress; // points past the end, like vector::end() etc

        // For proper behaviour alignment is expected to be a power of 2
        void* AllocateForward(unsigned int len, unsigned int alignment);
        void* AllocateBackward(unsigned int len, unsigned int alignment);
    } range;
};