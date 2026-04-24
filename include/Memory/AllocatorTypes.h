#pragma once

#include "AllocatorBase.h"
#include "UnusedSignedAllocator.h"
#include "HMRFAllocator.h"
#include "HPXEAllocator.h"

// The order of functions in the binary is the reverse of that of
// the vtables, so any ordering is a bit silly. Here they're ordered by
// address of vtable, as that way type A is the much simpler one.

// Never used.
// vtable stored at 020e9160.
// That is, the allocation function is at 020afe04 and the
// free function is at 020afe24.
// This allocator appeals to an externally held UnusedSignedAllocator, which
// just has a linked list of equal-sized blocks and allocates by removing one.
struct AllocatorTypeUnused : public AllocatorBase // nonvirtual inheritance!
{
    UnusedSignedAllocator* pSignedAllocator;

    // Static to minimise function pointer messiness
    static void* Allocate(AllocatorBase* base, unsigned int len);
    static void Free(AllocatorBase* base, void* data);

    static const VTable s_vtable;
};

// vtable stored at 020e9168.
// That is, the allocation function is at 020afdec, and the 
// free function is at 020afe00.
// This allocator appeals to an externally held HMRFAllocator, which
// implements two-sided arena allocation on a big block.
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
    static void* Allocate(AllocatorBase* base, unsigned int len);
    static void Free(AllocatorBase* base, void* data);

    static const VTable s_vtable;
};

// vtable stored at 020e9170.
// That is, the allocation function is at 020afdc8, and the 
// free function is at 020afddc.
// This allocator appeals to an externally held HPXEAllocator, which
// implements allocation by holding a linked list of free blocks and a
// linked list of used blocks and coalescing adjacent free blocks.
struct AllocatorTypeB : public AllocatorBase // nonvirtual inheritance
{
    HPXEAllocator* pHPXEAllocator;
    int alignmentAndDir;
    int unknown;

    // Static to minimise function pointer messiness
    static void* Allocate(AllocatorBase* base, unsigned int len);
    static void Free(AllocatorBase* base, void* data);

    static const VTable s_vtable;
};

// There seems to be a fourth type of allocator, with a vtable at 020e9178.
// It appears that it is never used in the context of polymorphic allocators,
// but does see some use elsewhere. It also appears to be much more complicated
// and its underlying functions are in a different region of memory
// (func_020c8854 to allocate, func_020c985c to free) so I'm leaving it out 
// for now.