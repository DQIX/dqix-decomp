#include "Memory/AllocatorUnion.h"

// USA: func_020afe6c
// JPN: func_020b1938
void* AllocatorUnion::Allocate(unsigned int len)
{
    return versions.base.pVTable->allocate(&versions.base, len);
}

// USA: func_020afe80
// JPN: func_020b194c
void AllocatorUnion::Free(void* data)
{
    versions.base.pVTable->free(&versions.base, data);
}

// USA: func_020afe94
// JPN: func_020b1960
void AllocatorUnion::InitializeTypeB(HPXEAllocator* pAlloc, int align)
{
    versions.typeB.pVTable = &AllocatorTypeB::s_vtable;
    versions.typeB.pHPXEAllocator = pAlloc;
    versions.typeB.alignmentAndDir = align;
    versions.typeB.unknown = 0;
}

// USA: func_020afeac
// JPN: func_020b1978
void AllocatorUnion::InitializeTypeA(HMRFAllocator* pAlloc, int align)
{
    versions.typeA.pVTable = &AllocatorTypeA::s_vtable;
    versions.typeA.pHMRFAllocator = pAlloc;
    versions.typeA.alignmentAndDir = align;
    versions.typeA.unknown = 0;
}