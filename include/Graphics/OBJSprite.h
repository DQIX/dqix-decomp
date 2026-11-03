#pragma once

#include "../NitroSDK/Graphics2D/Graphics2D.h"
#include "../Memory/SafeAllocator.h"
#include "Vector.h"

// sizeof == 0x54.
// A 2D sprite rendered via the OBJ system
class OBJSprite
{
public:
    // sizeof == 0x28
    struct Metatile
    {
        Nitro2DCellBank::Metatile nitroData_0;
        OAMEntry* oamEntries_8;
        fix32_t unknown_c; // scale x
        fix32_t unknown_10; // scale y
        Vector2fix position_14;
        // index into rotation array
        unsigned short unknown_1c;
        short unknown_1e;
        unsigned short rotScaleDataIndex_20;
        unsigned char destinationIndex_22;
        bool useTransform_23;
        bool doubleSize_24;
        char paletteIndex_25;
        char priority_26;

        void Reset();
    };

    struct AnimationFrame
    {
        unsigned short unknown_0;
        unsigned short unknown_2;

        inline void Reset()
        {
            unknown_0 = 0;
            unknown_2 = 0;
        }
    };

    // sizeof == 0x18
    struct Animation
    {
        int unknown_0;
        short unknown_4;
        short unknown_6;
        short bufferIndex_8;
        unsigned short numFrames_a;
        AnimationFrame* frames_c;
        short unknown_10;
        short unknown_12;
        unsigned char unknown_14;
        char unknown_15;

        inline void Reset()
        {
            unknown_0 = 0;
            unknown_6 = 0;
            unknown_4 = 0;
            bufferIndex_8 = 0;
            numFrames_a = 0;
            frames_c = NULL;
            unknown_10 = 0;
            unknown_12 = 0;
            unknown_14 = 0xff;
            unknown_15 = 1;
        }
    };

    struct AnimationList
    {
        Animation* data;
        unsigned short count;
    };

    Nitro2DCharacterLoadDesc charLoad_0_;
    Nitro2DPaletteLoadDesc paletteLoad_24_;
    int unknown_38_;
    AnimationList* animations_3c_;
    Metatile* metatiles_40_;
    int unknown_44_;
    int unknown_48_;
    unsigned short numMetatiles_4c_;
    short unknown_4e_;
    // 0 = main engine, 1 = sub engine. might be bool
    unsigned char unknown_50_;

    void Reset();
    void Destroy(); // does nothing but is probably Destroy given how it's used

    // Note: the file data is modified to turn offsets into pointers so it must
    // be kept in memory while the sprite is being used
    bool LoadFromPAC(void* data, unsigned int length, SafeAllocator* alloc);

    // Note: the file data is modified to turn offsets into pointers so it must
    // be kept in memory while the sprite is being used
    bool LoadFromNitroFile(void* data, unsigned int length, SafeAllocator* alloc);

    bool LoadNCER(void* data, unsigned int length, SafeAllocator* alloc);
    bool LoadNANR(void* data, unsigned int length, SafeAllocator* alloc);
    bool LoadNCGR(void* data, unsigned int length);
    bool LoadNCLR(void* data, unsigned int length);

    void DrawMetatile(Metatile* metatile);

    void Compute2x2Matrices(Matrix2x2* outRotScale, Matrix2x2* outRotInvScale, const Metatile* metatile);

    int GetMetatileObjMode(const Metatile* metatile) const;
};

// returns false on success, true on fail?!
bool GetCharBlockFromNitroFile(Nitro2DCharacter** out, void* data, unsigned int length);