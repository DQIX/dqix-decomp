#pragma once

#include "../NitroSDK/Graphics2D/Graphics2D.h"
#include "../Memory/SafeAllocator.h"
#include "Vector.h"

// sizeof == 0x54.
// A 2D sprite rendered via the OBJ system
class OBJSpriteSet
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
        unsigned short normalizedAngle_1c;
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
        unsigned short duration_0;
        unsigned short metatileIndex_2;

        inline void Reset()
        {
            duration_0 = 0;
            metatileIndex_2 = 0;
        }
    };

    // sizeof == 0x18
    struct Sprite
    {
        int animationType_0; // 1 = stop at end, 2 = loop
        short unknown_4;
        short unknown_6;
        unsigned short id_8;
        unsigned short numFrames_a;
        AnimationFrame* frames_c;
        unsigned short currentFrame_10;
        unsigned short frameTime_12;
        unsigned char destinationIndex_14;
        // bit 0: ??? maybe stopped?
        // bit 1: is playing?
        // bit 2: ???
        unsigned char flags_15;

        inline void Reset()
        {
            animationType_0 = 0;
            unknown_6 = 0;
            unknown_4 = 0;
            id_8 = 0;
            numFrames_a = 0;
            frames_c = NULL;
            currentFrame_10 = 0;
            frameTime_12 = 0;
            destinationIndex_14 = 0xff;
            flags_15 = 1;
        }
    };

    struct List
    {
        Sprite* sprites;
        unsigned short count;

        void Reset();
        void Destroy();
        Sprite* GetSprite(unsigned int id);
        void SetSpriteOBJIndex(unsigned short spriteID, unsigned char to);
        void Update(unsigned char spriteID, int numTicks);
        void UpdateAll(int numTicks);

        void PlaySpriteAnimation(unsigned int spriteID);
        void StopSpriteAnimation(unsigned int spriteID);
    };

    Nitro2DCharacterLoadDesc charLoad_0_;
    Nitro2DPaletteLoadDesc paletteLoad_24_;
    int alignmentBoundaryType_38_; // matches DISPCNT bits 20-21?
    List* sprites_3c_;
    Metatile* metatiles_40_;
    int charVRAMLoadOffset_44_;
    int usedObjVRAMEnd_48_;
    unsigned short numMetatiles_4c_;
    short numMetatiles_4e_;
    // 0 = main engine, 1 = sub engine. might be bool
    unsigned char engine_50_;

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
    void DrawSprite(unsigned char id);
    void DrawAll();

    void AssignOBJIndices(Metatile* metatiles, unsigned short numMetatiles, unsigned char firstIndex);

    unsigned short GetMetatilePalette(const Metatile* metatile) const;

    // if transformEnabled == false, then doubleSize == true means disable/hide
    // (but this is only ever used with true, true)
    void SetMetatileTransformFlags(Metatile* metatile, bool transformEnabled, bool doubleSize);
    void SetMetatilePriority(Metatile* metatile, unsigned char priority);
    void SetMetatilePalette(Metatile* metatile, unsigned short palette);
    void SetMetatileOBJMode(Metatile* metatile, unsigned char mode);

    bool SetOAMWord(Metatile* metatile, unsigned int wordIdx, unsigned short clearMask,
        unsigned short setShift, unsigned short setMask);
    void Compute2x2Matrices(Matrix2x2* outRotScale, Matrix2x2* outRotInvScale, const Metatile* metatile);

    int GetMetatileObjMode(const Metatile* metatile) const;
    unsigned int GetMetatilePaletteInternal(const Metatile* metatile) const;
};

// returns false on success, true on fail?!
bool GetCharBlockFromNitroFile(Nitro2DCharacter** out, void* data, unsigned int length);