#pragma once

#include "Common.h"

struct Nitro2DPalette
{
    uint32_t textureFormat_0;
    uint32_t unknown_4;
    uint32_t paletteDataSize_8;
    uint8_t* pPaletteData_c;
};

struct Nitro2DPaletteCMP
{
    uint16_t numPalettes_0;
    uint16_t unknown_2;
    uint16_t* pPaletteIDs_4;
};

struct Nitro2DPaletteLoadOutput
{
    uint32_t textureFormat_0;
    uint32_t unknown_4;
    uint32_t loadOffsets_8[3];
};

CBool Nitro2D_GetPalette(void* fileData, Nitro2DPalette** output);
CBool Nitro2D_GetPaletteCMP(void* fileData, Nitro2DPaletteCMP** output);

void Nitro2D_LoadPaletteToVRAM(Nitro2DPalette* palette, unsigned int vramOffset, int vramType, Nitro2DPaletteLoadOutput* output);
void Nitro2D_LoadPaletteWithCMPToVRAM(Nitro2DPalette* palette, Nitro2DPaletteCMP* paletteCMP,
    unsigned int vramOffset, int vramType, Nitro2DPaletteLoadOutput* output);