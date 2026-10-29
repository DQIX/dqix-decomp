#pragma once

#include <std_library_functions.h>

#if defined(jpn)
#define data_020f1ee8 data_020f2054
#define data_020f1eec data_020f2058
#define data_020f1ef0 data_020f205c
#define data_020f1ef4 data_020f2060
#endif

void InitializeTextureImageAllocation(unsigned int numBanks, CBool setAllocFunctions);
unsigned int FrameAllocateTextureImageVRAM(unsigned int requestedSize, CBool compressed, int unused);
int FrameFreeTextureImageVRAM(unsigned int);

void SaveTextureImageAllocationState(unsigned int* output);
void RestoreTextureImageAllocationState(const unsigned int* input);

void InitializeTexturePaletteVRAMAllocation(unsigned int size, CBool setAllocFunctions);
unsigned int FrameAllocateTexturePaletteVRAM(unsigned int size, CBool eightByte, int allocDir);
int FrameFreeTexturePaletteVRAM(unsigned int);

void SaveTexturePaletteAllocationState(unsigned int* pRange);
void RestoreTexturePaletteAllocationState(const unsigned int* pRange);

typedef unsigned int (*TextureImageVRAMAllocProc)(unsigned int size, CBool compressed, int unused);
typedef int (*TextureImageVRAMFreeProc)(unsigned int handle);
typedef unsigned int (*TexturePaletteVRAMAllocProc)(unsigned int size, CBool eightByteAlign, int allocDir);
typedef int (*TexturePaletteVRAMFreeProc)(unsigned int handle);

extern TexturePaletteVRAMAllocProc g_pfnAllocTexturePalette;
extern TexturePaletteVRAMFreeProc g_pfnFreeTexturePalette;
extern TextureImageVRAMAllocProc g_pfnAllocTextureImage;
extern TextureImageVRAMFreeProc g_pfnFreeTextureImage;