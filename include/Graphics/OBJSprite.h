#pragma once

#include "../NitroSDK/Graphics2D/Graphics2D.h"
#include "../Memory/SafeAllocator.h"

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
        int unknown_c;
        int unknown_10;
        int unknown_14;
        int unknown_18;
        short unknown_1c;
        short unknown_1e;
        short unknown_20;
        unsigned char unknown_22;
        unsigned char unknown_23;
        unsigned char unknown_24;
        char unknown_25;
        char unknown_26;

        void Reset();
    };

    Nitro2DCharacterLoadOutput charLoad_0_;
    Nitro2DPaletteLoadOutput paletteLoad_24_;
    int unknown_38_;
    int unknown_3c_;
    Metatile* metatiles_40_;
    int unknown_44_;
    int unknown_48_;
    unsigned short numMetatiles_4c_;
    short unknown_4e_;
    bool unknown_50_;

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
};

// returns false on success, true on fail?!
bool GetCharBlockFromNitroFile(Nitro2DCharacter** out, void* data, unsigned int length);