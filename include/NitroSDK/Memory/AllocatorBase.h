#pragma once

// In essence we want a virtual class: the binary is clearly using a vtable. However
// we'll want to hold an instance of this class locally within another struct, instead
// of just holding a pointer to an external one. We implement this as a union,
// but this compiler freaks out doing that, so we just implement the vtable
// manually. 
// (For what it's worth, this compiler also doesn't seem to recognise placement
// new, which would have also worked by declaring a suitably sized buffer).
struct AllocatorBase
{
    struct VTable
    {
        typedef void* (*PFNAllocate)(AllocatorBase*, unsigned int);
        typedef void (*PFNFree)(AllocatorBase*, void*);
        
        PFNAllocate allocate;
        PFNFree free;
    };

    const VTable* pVTable;
};