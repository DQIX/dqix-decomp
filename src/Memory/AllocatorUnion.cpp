#include "Memory/AllocatorUnion.h"

void* AllocatorUnion::Allocate(unsigned int len)
{
    return versions.base.pVTable->allocate(&versions.base, len);
}

void AllocatorUnion::Free(void* data)
{
    versions.base.pVTable->free(&versions.base, data);
}

void AllocatorUnion::InitializeTypeB(void* pAlloc, int unknown)
{
    versions.typeB.pVTable = &AllocatorTypeB::s_vtable;
    versions.typeB.pUnderlyingAllocator = pAlloc;
    versions.typeB.unknown[0] = unknown;
    versions.typeB.unknown[1] = 0;
}

void AllocatorUnion::InitializeTypeA(void* pAlloc, int align)
{
    versions.typeA.pVTable = &AllocatorTypeA::s_vtable;
    versions.typeA.pArenaAllocator = pAlloc;
    versions.typeA.alignmentAndDir = align;
    versions.typeA.unknown = 0;
}