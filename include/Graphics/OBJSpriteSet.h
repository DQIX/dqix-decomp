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
    // represents one frame of one sprite
    struct Metatile
    {
        Nitro2DCellBank::Metatile nitroData;
        OAMEntry* oamEntries;
        Vector2fix scale;
        Vector2fix position;
        // index into rotation array
        unsigned short normalizedAngle;
        short id;
        // which of the 32 4-tuples of words at OAM+06, OAM+0e etc to use
        unsigned short rotScaleDataIndex;
        unsigned char destinationObjIndex;
        bool useTransform;
        bool doubleSize;
        char paletteIndex;
        char priority;

        void Reset();
    };

    struct AnimationFrame
    {
        unsigned short duration;
        unsigned short metatileIndex;

        inline void Reset()
        {
            duration = 0;
            metatileIndex = 0;
        }
    };

    // sizeof == 0x18
    struct Sprite
    {
        int animationType; // 1 = stop at end, 2 = loop
        short drawX;
        short drawY;
        unsigned short id;
        unsigned short numFrames;
        AnimationFrame* frames;
        unsigned short currentFrame;
        unsigned short frameTime;
        unsigned char destinationObjIndex;
        // bit 0: ??? maybe stopped?
        // bit 1: is playing?
        // bit 2: ???
        // bit 3: ??? maybe hidden?
        unsigned char flags;

        inline void Reset()
        {
            animationType = 0;
            drawX = drawY = 0;
            id = 0;
            numFrames = 0;
            frames = NULL;
            currentFrame = 0;
            frameTime = 0;
            destinationObjIndex = 0xff;
            flags = 1;
        }
    };

    struct List
    {
        Sprite* sprites_;
        unsigned short count_;

        void Reset();
        void Destroy();
        Sprite* GetSprite(unsigned int id);
        void SetSpriteOBJIndex(unsigned short spriteID, unsigned char to);
        void Update(unsigned char spriteID, int numTicks);
        void UpdateAll(int numTicks);

        void PlaySpriteAnimation(unsigned int spriteID);
        // only present in jpn version, used in ov003? I couldn't trigger it
        void MaybePauseSpriteAnimation(unsigned int spriteID);
        void StopSpriteAnimation(unsigned int spriteID);
    };

    Nitro2DCharacterLoadDesc charLoad_;
    Nitro2DPaletteLoadDesc paletteLoad_;
    int alignmentBoundaryType_; // matches DISPCNT bits 20-21?
    List* sprites_;
    Metatile* metatiles_;
    int charVRAMLoadOffset_;
    int usedObjVRAMEnd_;
    unsigned short numMetatiles_;
    unsigned short numMetatilesCopy_; // only ever written to?
    // 0 = main engine, 1 = sub engine. might be bool
    unsigned char engine_;

    void Reset();
    void Destroy(); // does nothing but is probably Destroy given how it's used

    // Note: the file data is modified to turn offsets into pointers so it must
    // be kept in memory while the sprite is being used
    // returns true on fail, false on success?!
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

bool GetCharBlockFromNitroFile(Nitro2DCharacter** out, void* data, unsigned int length);