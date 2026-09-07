#pragma once

#include "../Memory/SafeAllocator.h"
#include "../Graphics/Vector.h"

// sizeof == 0x58
struct Zone3D_BMDJStruct
{
    struct StructSizeC
    {
        short maybeID;
        uint8_t unk_2;
        uint8_t unk_3;
        char* name;
        StructSizeC* pNext;
    };

    struct StructSize20
    {
        short maybeID;
        short unk_2;
        short unk_4;
        unsigned short flags_6;
        fix32_t unk_8[3]; // vector?
        // it feels like these should be fix16, but they're accessed as unsigned
        unsigned short unk_14;
        unsigned short unk_16;
        unsigned short unk_18;
        unsigned short unk_1a;
        unsigned short unk_1c;
        unsigned short unk_1e;
    };

    int unknown_0_;
    struct ScriptData
    {
        StructSizeC* ptr_0; // allocated by opcode 6a
        int counter_4; // size of array at 0x0
        int capacity_8; // capacity of allocation at 0x0
        StructSize20* ptr_c; // see func_0201f214 for initialization
        int counter_10; // size of array at 0xc
        int capacity_14; // capacity of allocation at 0xc
        char* strings_18[8];
        unsigned char stringCount_38;
        char pad_39[3];

        void Reset();
        void Load(SafeAllocator* alloc, const void* script, unsigned int length);
        void InsertStructC(const StructSizeC& source);
        StructSizeC* GetStructC(int idx);
        StructSizeC* GetStructCByID(int id);
        int FindStructCByID(int id) const;

        void AllocateStruct20Array(int count, SafeAllocator* alloc);
        void InsertStruct20(const StructSize20& source);
        StructSize20* GetStruct20(int idx);
        StructSize20* GetStruct20ByID(int id);

        void StoreString(const char* str, SafeAllocator* alloc);
        const char* GetString(int idx) const;

    } scriptData_;
    void* ptr_40; // array of length scriptData_.counter_4, stride = 0x18 (func_02012ff0)
    void* ptr_44; // array of length scriptData_.counter_10, stride = 0x70 (func_020181fc)
    Vector3i vec_48_;
    Zone3D_BMDJStruct* pNext_;
};