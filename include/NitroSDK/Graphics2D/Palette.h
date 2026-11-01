#pragma once

#include "Common.h"

struct Nitro2DPalette
{
    uint32_t colorDepth_0;
    uint32_t unknown_4;
    uint32_t paletteDataSize_8;
    uint16_t* pPaletteData_c;
};

CBool Nitro2D_GetPalette(void* fileData, Nitro2DPalette** output);