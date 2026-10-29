#include "NitroSDK/VRAMAllocation.h"

static unsigned int InvalidTextureVRAMAlloc(unsigned int, CBool, int)
{
    return 0;
}

static int InvalidTextureVRAMFree(unsigned int)
{
    return -1;
}

static unsigned int InvalidPaletteVRAMAlloc(unsigned int, CBool, int)
{
    return 0;
}

static int InvalidPaletteVRAMFree(unsigned int)
{
    return -1;
}

TextureImageVRAMAllocProc g_pfnAllocTextureImage = &InvalidTextureVRAMAlloc;
TextureImageVRAMFreeProc g_pfnFreeTextureImage = &InvalidTextureVRAMFree;
TexturePaletteVRAMAllocProc g_pfnAllocTexturePalette = &InvalidPaletteVRAMAlloc;
TexturePaletteVRAMFreeProc g_pfnFreeTexturePalette = &InvalidPaletteVRAMFree;
