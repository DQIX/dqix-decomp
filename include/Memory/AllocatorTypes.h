#pragma once

#include "AllocatorBase.h"
#include "HMRFAllocator.h"
#include "HPXEAllocator.h"

// The order of functions in the binary is the reverse of that of
// the vtables, so any ordering is a bit silly. Here they're ordered by
// address of vtable, as that way type A is the much simpler one.

// vtable stored at 020e9168.
// That is, the allocation function is at 020afdec, and the 
// free function is at 020afe00.
// This allocator just appeals to an externally held two-sided
// arena allocator (i.e. that can allocate forwards from the start of its
// block or backwards from the end).
struct AllocatorTypeA : public AllocatorBase // nonvirtual inheritance!
{
    HMRFAllocator* pHMRFAllocator; // type will be determined later
    // If this is positive, the allocation will go forwards, and if negative
    // it will go backwards. The absolute value is a power of 2 indicating
    // the alignment of each allocated item. (In practice, this always seems
    // to be +4).
    int alignmentAndDir;
    int unknown;

    // Static to minimise function pointer messiness
    static void* Allocate(AllocatorBase* self, unsigned int size);
    static void Free(AllocatorBase* self, void* data);

    static const VTable s_vtable;
};

// vtable stored at 020e9170.
// That is, the allocation function is at 020afdc8, and the 
// free function is at 020afddc.
// Like the type A allocator, it just appeals to a separately held allocator.
// This one is more complicated, seemingly doing some linked list shenanigans.
struct AllocatorTypeB : public AllocatorBase // nonvirtual inheritance
{
    HPXEAllocator* pHPXEAllocator;
    int alignmentAndDir;
    int unknown;

    // Static to minimise function pointer messiness
    static void* Allocate(AllocatorBase* self, unsigned int size);
    static void Free(AllocatorBase* self, void* data);

    static const VTable s_vtable;
};