#pragma once

#include "Common.h"

struct Nitro2DPalette
{
    uint32_t textureFormat;
    CBool useExtendedPalette;
    uint32_t paletteDataSize;
    uint8_t* pPaletteData;
};

struct Nitro2DPaletteCMP
{
    uint16_t numPalettes;
    char unk_2[2];
    uint16_t* pPaletteIDs;
};

struct Nitro2DPaletteLoadDesc
{
    uint32_t textureFormat;
    CBool useExtendedPalette;
    uint32_t loadOffsets[3];
};

CBool Nitro2D_GetPalette(void* fileData, Nitro2DPalette** output);
CBool Nitro2D_GetPaletteCMP(void* fileData, Nitro2DPaletteCMP** output);

void Nitro2D_ClearPaletteLoadDesc(Nitro2DPaletteLoadDesc* desc);
// vram type: 0 = texture palette, 1 = main engine obj palette (standard/extended),
// 2 = sub engine obj palette (standard/extended)
void Nitro2D_LoadPaletteToVRAM(Nitro2DPalette* palette, unsigned int vramOffset, int vramType, Nitro2DPaletteLoadDesc* output);
// vram type: 0 = texture palette, 1 = main engine obj palette (standard/extended),
// 2 = sub engine obj palette (standard/extended)
void Nitro2D_LoadPaletteWithCMPToVRAM(Nitro2DPalette* palette, Nitro2DPaletteCMP* paletteCMP,
    unsigned int vramOffset, int vramType, Nitro2DPaletteLoadDesc* output);