#pragma once

#include "itcm/VRAMStaging.h"
#include "NSBXX/NSBXX.h"

void LockStagedTextureVRAMCopying();
void UnlockStagedTextureVRAMCopying();
void* AllocateVRAMStagingMemory(unsigned int length);
void FreeVRAMStagingMemory(const void* allocation);
void StageMemoryToVRAM(VRAMSubregion subregion, const void* data, unsigned int offset,
    unsigned int length, bool highPriority, bool allocateStagingSpace);
int StageTexFilePaletteData(volatile NSBXXTex* texture, bool highPriority);
int StageTexFileImageData(volatile NSBXXTex* texture, bool highPriority);
bool CancelVRAMStagingOperation(int id);
void UpdateVRAMStagingVRAMBanks();