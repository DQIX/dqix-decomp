#include "Graphics/VRAMStaging.h"
#include <globaldefs.h>

VRAMStagingManager g_stagingManagerInstance;
unsigned char g_vramStagingBuffer[0x5000];

void LockStagedTextureVRAMCopying()
{
    g_stagingManagerInstance.textureLockMask_ <<= 1;
    g_stagingManagerInstance.textureLockMask_ |= 1;
}

void UnlockStagedTextureVRAMCopying()
{
    g_stagingManagerInstance.textureLockMask_ >>= 1;
}

void* AllocateVRAMStagingMemory(unsigned int length)
{
    return g_stagingManagerInstance.AllocateInStagingSpace(length);
}

void FreeVRAMStagingMemory(const void* allocation)
{
    g_stagingManagerInstance.FreeStagingSpaceAllocation(allocation);
}

void StageMemoryToVRAM(VRAMSubregion subregion, const void* data, unsigned int offset,
    unsigned int length, bool highPriority, bool allocateStagingSpace)
{
    g_stagingManagerInstance.Stage(subregion, data, offset, length, highPriority, allocateStagingSpace);
}

int StageTexFilePaletteData(volatile NSBXXTex* texture, bool highPriority)
{
    if (texture == NULL)
        return -1;
        
    texture->maybeBlock4Flags_32_ |= (1 << 0);

    int size = texture->block4NumEightBytes_;
    size <<= 3;
    int offset = (unsigned short)texture->block4VRAMLoadOffset_ << 3;
    const void* src = (const void*)((intptr_t)texture + texture->block4Offset_);

    return g_stagingManagerInstance.Stage(VRAMSubregion_TexturePalette,
        src, offset, size, highPriority, false);
}

int StageTexFileImageData(volatile NSBXXTex* texture, bool highPriority)
{
    if (texture == NULL)
        return -1;
        
    texture->maybeBlock1Flags_10_ |= (1 << 0);

    int size = texture->block1NumEightBytes_;
    size <<= 3;
    int offset = (unsigned short)texture->block1VRAMLoadOffset_ << 3;
    const void* src = (const void*)((intptr_t)texture + texture->block1Offset_);

    return g_stagingManagerInstance.Stage(VRAMSubregion_TextureImage,
        src, offset, size, highPriority, false);
}

bool CancelVRAMStagingOperation(int id)
{
    return g_stagingManagerInstance.CancelTaskByID(id);
}

void UpdateVRAMStagingVRAMBanks()
{
    g_stagingManagerInstance.UpdateBanks();
}