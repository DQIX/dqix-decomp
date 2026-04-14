#pragma once

#include <globaldefs.h>
#include "DoubleEndedBlock.h"

class DoubleEndedArenaAllocator
{
public:
    char unknown[0x20];
    DoubleEndedBlock block;

    void* Allocate(unsigned int len, int alignAndDirection);
};