#include "Memory/AllocatorUnion.h"

// USA: func_020afe6c
void* AllocatorUnion::Allocate(unsigned int len)
{
    return versions.base.pVTable->allocate(&versions.base, len);
}

// USA: func_020afe80
void AllocatorUnion::Free(void* data)
{
    versions.base.pVTable->free(&versions.base, data);
}

// USA: func_020afe94
void AllocatorUnion::InitializeTypeB(HPXEAllocator* pAlloc, int align)
{
    versions.typeB.pVTable = &AllocatorTypeB::s_vtable;
    versions.typeB.pHPXEAllocator = pAlloc;
    versions.typeB.alignmentAndDir = align;
    versions.typeB.unknown = 0;
}

// USA: func_020afec4
void AllocatorUnion::InitializeTypeA(DoubleEndedArenaAllocator* pAlloc, int align)
{
    versions.typeA.pVTable = &AllocatorTypeA::s_vtable;
    versions.typeA.pArenaAllocator = pAlloc;
    versions.typeA.alignmentAndDir = align;
    versions.typeA.unknown = 0;
}