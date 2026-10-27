#pragma once

#include <std_library_functions.h>

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

extern TexturePaletteVRAMAllocProc data_020f1ef0;
extern TexturePaletteVRAMFreeProc data_020f1ef4;
extern TextureImageVRAMAllocProc data_020f1ee8;
extern TextureImageVRAMFreeProc data_020f1eec;