#pragma once

#include "Common.h"

// signature 'CEBK'
struct Nitro2DCellBank
{
    struct OBJAttributes;
    struct Metatile
    {
        uint16_t numObjs_0;
        uint16_t unknown_2;
        OBJAttributes* pAttributes_4;
    };
    struct MetatileEXT
    {
        Metatile base;
        char unk_8[8];
    };

    struct OBJAttributes
    {
        uint16_t first;
        uint16_t second;
        uint16_t third;
    };

    struct SubstructC
    {
        char unk_0[4];
        void* unknown_4;
    };

    // None of the game files seem to use this, and GBATek has no 
    // documentation for it
    struct Substruct14
    {
        char header_0[8]; // might be InnerFileHeader?
        struct Interior
        {
            uint16_t numPointers_0;
            char unk_2[2];
            void** pointers_4;
        } interior;
    };

    uint16_t numMetatiles_0;
    uint16_t metatileEntrySize_2; // 0 = 8 bytes, 1 = 16 bytes
    Metatile* pMetatiles_4;
    char unk_8[4];
    SubstructC* unknown_c;
    char unk_10[4];
    Substruct14* unknown_14;
};

CBool Nitro2D_GetCellBank(void* fileData, Nitro2DCellBank** output);
Nitro2DCellBank::Metatile* Nitro2D_GetCellBankMetatile(Nitro2DCellBank* bank, unsigned int idx);