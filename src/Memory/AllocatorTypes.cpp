#include "Memory/AllocatorTypes.h"

const AllocatorBase::VTable AllocatorTypeA::s_vtable = {
    &AllocatorTypeA::Allocate,
    &AllocatorTypeA::Free
};

const AllocatorBase::VTable AllocatorTypeB::s_vtable = {
    &AllocatorTypeB::Allocate,
    &AllocatorTypeB::Free
};

extern "C"
{
    void* func_020af748(void* allocator, unsigned int len, int unknown);
    void func_020af778(void* allocator, void* data);
}

void* AllocatorTypeB::Allocate(AllocatorBase* base, unsigned int len)
{
    AllocatorTypeB* self = static_cast<AllocatorTypeB*>(base);
    return func_020af748(self->pUnderlyingAllocator, len, self->unknown[0]);
}

void AllocatorTypeB::Free(AllocatorBase* base, void* data)
{
    AllocatorTypeB* self = static_cast<AllocatorTypeB*>(base);
    func_020af778(self->pUnderlyingAllocator, data);
}


extern "C" void* func_020af9b8(void* arenaAllocator, unsigned int len, int align);

void* AllocatorTypeA::Allocate(AllocatorBase* base, unsigned int len)
{
    AllocatorTypeA* self = static_cast<AllocatorTypeA*>(base);
    return func_020af9b8(self->pArenaAllocator, len, self->alignmentAndDir);
}

void AllocatorTypeA::Free(AllocatorBase* base, void* data)
{
    // Don't do anything
}