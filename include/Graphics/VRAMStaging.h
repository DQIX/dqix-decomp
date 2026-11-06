#pragma once

#include "itcm/VRAMStaging.h"
#include "../NitroSDK/Graphics3D/Graphics3D.h"

void LockStagedTextureVRAMCopying();
void UnlockStagedTextureVRAMCopying();
void* AllocateVRAMStagingMemory(unsigned int length);
void FreeVRAMStagingMemory(const void* allocation);
void StageMemoryToVRAM(VRAMSubregion subregion, const void* data, unsigned int offset,
    unsigned int length, bool highPriority, bool allocateStagingSpace);
int StageTexFilePaletteData(volatile Nitro3DTextureSet* texture, bool highPriority);
int StageTexFileImageData(volatile Nitro3DTextureSet* texture, bool highPriority);
bool CancelVRAMStagingOperation(int id);
void UpdateVRAMStagingVRAMBanks();