#pragma once

#include <globaldefs.h>
#include "AllocatorBase.h"
#include "AllocatorTypes.h"

class AllocatorUnion
{
public:
    union 
    {
        AllocatorBase base;
        AllocatorTypeA typeA;
        AllocatorTypeB typeB;
    } versions;
    
    void* Allocate(unsigned int len);
    void Free(void* data);

    void InitializeTypeB(void* pAlloc, int unknown);
    void InitializeTypeA(void* pArenaAlloc, int alignAndDir);
};