#include "Memory/AllocatorTypes.h"

// USA: data_020e9168
const AllocatorBase::VTable AllocatorTypeA::s_vtable = {
    &AllocatorTypeA::Allocate,
    &AllocatorTypeA::Free
};

// USA: data_020e9170
const AllocatorBase::VTable AllocatorTypeB::s_vtable = {
    &AllocatorTypeB::Allocate,
    &AllocatorTypeB::Free
};

// USA: func_020afdc8
void* AllocatorTypeB::Allocate(AllocatorBase* base, unsigned int len)
{
    AllocatorTypeB* self = static_cast<AllocatorTypeB*>(base);
    return self->pHPXEAllocator->Allocate(len, self->alignmentAndDir);
}

// USA: func_020afddc
void AllocatorTypeB::Free(AllocatorBase* base, void* data)
{
    AllocatorTypeB* self = static_cast<AllocatorTypeB*>(base);
    self->pHPXEAllocator->Free(data);
}

// USA: func_020afdec
void* AllocatorTypeA::Allocate(AllocatorBase* base, unsigned int len)
{
    AllocatorTypeA* self = static_cast<AllocatorTypeA*>(base);
    return self->pArenaAllocator->Allocate(len, self->alignmentAndDir);
}

// USA: func_020afe00
void AllocatorTypeA::Free(AllocatorBase* base, void* data)
{
    // Don't do anything
}