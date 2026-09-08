#pragma once

#include "../Graphics/Vector.h"
#include "../Memory/SafeAllocator.h"
#include "../System/Memory.h"
#include "../Grotto/Main/TileFeatures.h"

union WrappedVector3fix
{
    fix32_t entries[3];
    Vector3fix vec;
};

// I don't know what this is, but in practice it gets populated from a pair
// of vector3s (a, b). The resulting vector component is b - a normalized, and
// the scalar is the inner product of a with said normalized vector. The only
// thing I can think of that looks like this is a representation of a plane
// in 3D space...?
struct MaybeVector4fix
{
    WrappedVector3fix vec3;
    fix32_t w;

    // copy assignment operator, but it's explicitly defined?!
    // if we make it implicit, it gets placed too early in the TU
    // defining as operator= works but causes trouble on other compilers
    // when put in a union
    MaybeVector4fix& CopyFrom(const MaybeVector4fix& other);
};

class ZoneFeatures
{
public:
    struct Opcode64Entry
    {
        int unk_0;
        WrappedVector3fix vector_4;
        char string_10[0x10];
        char string_20[0x10];
        char string_30[0x10];
        int unknown_40[6];
    };

    struct Opcode66Entry
    {
        fix32_t unk_0[6];
        int unk_18[2];
        fix16_t unk_20; 
    };

    // Uses include stairs in grottos
    struct Opcode68Entry
    {
        int unk_0;
        int unk_4[6];
        int unk_1c;
        Vector3fix unk_20[4];
        short unk_50;
        short unk_52;
        int unk_54;
        int unk_58[3];
        int unk_64[2];
        int unk_6c;

        void Reset();
    };

    // Used by opcodes 6a, 6b, 73, 74
    // Uses include chests in grottos
    struct Opcode6aEntry
    {
        unsigned short unk_0;
        int maybeType;
        WrappedVector3fix unk_8;
        WrappedVector3fix unk_14;
        short unk_20;
        short unk_22;
        int unk_24;
        int unk_28;
        union
        {
            struct {
                unsigned char unk_0;
            } type0;
            struct {
                unsigned char unk_0;
                unsigned char unk_1;
                unsigned short unk_2_low : 4;
                unsigned short unk_2_high : 12;
                unsigned short maybeZone;
                unsigned short unk_6;
                unsigned short unk_8;
                char padding_a[2];
                Vector3fix vectors_c[4];
                fix16_t unk_3c;
                fix16_t unk_3e;
                fix32_t unk_40;
            } type2;
            struct {
                unsigned char unk_0;
            } type3;
            struct {
                unsigned char unk_0 : 7;
                unsigned char unk_0_high : 1;
                char unk_1;
                char unk_2[2];
                MaybeVector4fix vector_4;
                fix32_t distance_14;
                Vector3fix vector_18;
                Vector3fix vector_24;
                Vector3fix vector_30;
            } type4;
            struct {
                unsigned short unk_0;
                unsigned short unk_2;
                unsigned short unk_4;
            } type5;
            struct {
                unsigned short unk_0;
                unsigned short unk_2;
                unsigned char unk_4;
                unsigned char unk_5;
                unsigned short maybeZone;
                unsigned short unk_8;
                unsigned short unk_a;
                Vector3fix vector_c;
                Vector3fix vector_18;
                Vector3fix vector_24;
                Vector3fix vector_30;
                fix16_t unk_3c;
                fix16_t unk_3e;
                fix16_t unk_40;
                unsigned short unk_42;
            } type9;
            struct {
                unsigned short unk_0;
                fix16_t unk_2;
                Vector3fix vector_4;
                fix32_t unk_10;
                Vector3fix vector_14;
                fix16_t unk_20;
            } type10;
            struct {
                unsigned char subtype;
                unsigned char unk_1;
                unsigned char unk_2;
                char padding_3;
                union
                {
                    struct {
                        unsigned char unk_0;
                        unsigned char unk_1;
                        unsigned char unk_2;
                        unsigned char unk_3;
                        unsigned char unk_4;
                        unsigned char unk_5;
                    } subtype1;
                    struct {
                        unsigned char unk_0;
                        fix32_t unk_4;
                        fix32_t unk_8;
                        fix32_t unk_c;
                        unsigned char unk_10;
                    } subtype2;
                    struct {
                        unsigned char unk_0;
                        unsigned char unk_1;
                        fix32_t unk_4;
                        fix32_t unk_8;
                        fix32_t unk_c;
                        fix32_t unk_10;
                    } subtype3;
                };
            } type12;
        } unk_2c;
        Opcode6aEntry* pNext;

        void Reset();
    };

    struct Opcode7bEntry
    {
        WrappedVector3fix vector_0;
        WrappedVector3fix vector_c;
        fix16_t unk_18;
        char padding_1a[2];
        fix32_t minX;
        fix32_t minZ;
        fix32_t maxX;
        fix32_t maxZ;
    };

    Opcode64Entry* entries64_;
    int arraySize64_;
    int arrayCapacity64_;

    Opcode66Entry* entries66_;
    int arraySize66_;
    int arrayCapacity66_;

    Opcode68Entry* entries68_;
    int arraySize68_;
    int arrayCapacity68_;

    Opcode6aEntry* entries6a_;
    int arraySize6a_;
    int arrayCapacity6a_;

    TileFeaturePlacementData* grottoTileFeatureEntries_;
    int grottoTileFeatureEntryCount_;
    int grottoTileFeatureEntryCapacity_;

    Opcode6aEntry* entries6aByType_[13];

    Vector3fix vector_70_;
    fix16_t angle_7c_;
    // not sure what this is used for. In outdoor areas it tends to be blue
    // (0, 5, 30) but in caves it's black. It isn't background color though
    uint16_t color_7e_;

    unsigned short arraySize7b_;
    unsigned short arrayCapacity7b_;
    Opcode7bEntry* entries7b_;

    void Reset();

    void AllocateOpcode64Entries(int count, SafeAllocator* alloc);
    void AllocateGrottoTileFeaturePlacementEntries(int count, SafeAllocator* alloc);

    void LoadFromScript(SafeAllocator* alloc, const void* script, unsigned int length);
    Opcode64Entry* GetOpcode64Entry(int index);

    void CreateOpcode64Entry(const Opcode64Entry& data);
    void CreateGrottoTileFeaturePlacementEntry(const TileFeaturePlacementData& data);

    TileFeaturePlacementData* GetGrottoTileFeaturePlacementEntry(const char* name);

    void AllocateOpcode66Entries(int count, SafeAllocator* alloc);
    void CreateOpcode66Entry(const Opcode66Entry& data);

    void AllocateOpcode68Entries(int count, SafeAllocator* alloc);
    void CreateOpcode68Entry(const Opcode68Entry& data);

    void AllocateOpcode6aEntries(int count, SafeAllocator* alloc);
    Opcode6aEntry* CreateOpcode6aEntry(const Opcode6aEntry& data);
    // not real, just here for testing purposes
    Opcode6aEntry* CreateOpcode6aEntry(const Opcode6aEntry& data, int fakeArg);

    void SetOpcode7bAllocation(Opcode7bEntry* array, unsigned short capacity);
    void CreateOpcode7bEntry(const Opcode7bEntry& data);
};