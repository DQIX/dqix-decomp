#pragma once

#include "Common.h"

// signature 'CHAR'
struct Nitro2DCharacter
{
    uint16_t tileDataSizeKB_0;
    uint16_t unknown_2;
    uint32_t maybeColorDepth_4;
    // bits 4 and 20-21 of dispcnt
    uint32_t objTileMapping_8;
    char unk_c[4];
    uint32_t tileDataSizeBytes_10;
    void* pTileData_14;
};

struct Nitro2DCharacterLoadDesc
{
    uint32_t offsets_0[3];
    uint32_t unknown_c;
    uint32_t unknown_10;
    uint32_t unknown_14;
    uint32_t unknown_18;
    uint32_t unknown_1c;
    uint32_t objTileMapping_20;
};

CBool Nitro2D_GetCharacter(void* fileData, Nitro2DCharacter** output);

void Nitro2D_ClearCharacterLoadDesc(Nitro2DCharacterLoadDesc* desc);

// vramType: 0 = texture image, 1 = main obj image, 2 = sub obj image
void Nitro2D_LoadCharacterToVRAM(Nitro2DCharacter* character, unsigned int vramOffset, int vramType, Nitro2DCharacterLoadDesc* output);
// identical to the previous function
// vramType: 0 = texture image, 1 = main obj image, 2 = sub obj image
void Nitro2D_LoadCharacterToVRAM2(Nitro2DCharacter* character, unsigned int vramOffset, int vramType, Nitro2DCharacterLoadDesc* output);