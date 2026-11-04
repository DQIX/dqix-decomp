#pragma once

#include "Common.h"

// signature 'CHAR'
struct Nitro2DCharacter
{
    uint16_t unknown_0;
    uint16_t unknown_2;
    // might be texture format based on GBATek documentation?
    uint32_t unknown_4;
    // bits 4 and 20-21 of dispcnt
    uint32_t objTileMappingFlags;
    char unk_c[4];
    uint32_t tileDataSize;
    void* pTileData;
};

struct Nitro2DCharacterLoadDesc
{
    uint32_t loadOffsets[3];
    uint32_t unknown_c;
    uint32_t unknown_10;
    uint32_t unknown_14;
    uint32_t unknown_18;
    uint32_t unknown_1c;
    uint32_t objTileMapping;
};

CBool Nitro2D_GetCharacter(void* fileData, Nitro2DCharacter** output);

void Nitro2D_ClearCharacterLoadDesc(Nitro2DCharacterLoadDesc* desc);

// vramType: 0 = texture image, 1 = main obj image, 2 = sub obj image
void Nitro2D_LoadCharacterToVRAM(Nitro2DCharacter* character, unsigned int vramOffset, int vramType, Nitro2DCharacterLoadDesc* output);
// identical to the previous function
// vramType: 0 = texture image, 1 = main obj image, 2 = sub obj image
void Nitro2D_LoadCharacterToVRAM2(Nitro2DCharacter* character, unsigned int vramOffset, int vramType, Nitro2DCharacterLoadDesc* output);