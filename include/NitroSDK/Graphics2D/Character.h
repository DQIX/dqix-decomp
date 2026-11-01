#pragma once

#include "Common.h"

// signature 'CHAR'
struct Nitro2DCharacter
{
    uint16_t tileDataSizeKB_0;
    uint16_t unknown_2;
    uint32_t colorDepth_4;
    uint16_t unknown_8;
    uint16_t unknown_a;
    uint32_t unknown_c;
    uint32_t tileDataSizeBytes_10;
    void* pTileData_14;
};

CBool Nitro2D_GetCharacter(void* fileData, Nitro2DCharacter** output);