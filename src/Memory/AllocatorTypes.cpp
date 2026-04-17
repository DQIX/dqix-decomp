#include "Memory/AllocatorTypes.h"

// USA: data_020e9168
const AllocatorBase::VTable AllocatorTypeA::s_vtable = {
    &AllocatorTypeA::Allocate,
    &AllocatorTypeA::Free
};

// USA: data_020e9160.
// Why does the compiler place these symbols out of order?
// If we put this at the top, it goes at 9168 and type A goes at 9160.
const AllocatorBase::VTable AllocatorTypeUnused::s_vtable = {
    &AllocatorTypeUnused::Allocate,
    &AllocatorTypeUnused::Free
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
    return self->pHMRFAllocator->Allocate(len, self->alignmentAndDir);
}

// USA: func_020afe00
void AllocatorTypeA::Free(AllocatorBase* base, void* data)
{
    // Don't do anything
}

// USA: func_020afe04
void* AllocatorTypeUnused::Allocate(AllocatorBase* base, unsigned int len)
{
    AllocatorTypeUnused* self = static_cast<AllocatorTypeUnused*>(base);
    if (len > self->pSignedAllocator->blockSize)
        return NULL;
    
    return self->pSignedAllocator->Allocate();
}

// USA: func_020afe24
void AllocatorTypeUnused::Free(AllocatorBase* base, void* data)
{
    AllocatorTypeUnused* self = static_cast<AllocatorTypeUnused*>(base);
    self->pSignedAllocator->Free(data);
}