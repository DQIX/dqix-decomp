#pragma once

#include "../NitroSDK/VRAMAllocation.h"

// sizeof == 0x70.
// Helper class to manage VRAM allocations for texture image and palette data.
// The SDK provides some allocators for this which are basic frame allocators,
// and you free memory by taking a snapshot of the allocation state beforehand
// and then restoring to it.
// Functionally this class looks vaguely like an arena allocator, but it doesn't
// really check anything except that there's enough space at the very start. In
// particular if you have multiple of these and don't free/Reset() them in exactly
// the opposite order to creation/using them for allocation you can end up with
// garbage data
class TextureArena
{
public:
    unsigned int textureImageStateBase_[10];
    unsigned int textureImageStateCurrent_[10];
    unsigned int textureImageHandle_;
    unsigned int textureImageAllocatedSize_;

    unsigned int texturePaletteStateBase_[2];
    unsigned int texturePaletteStateCurrent_[2];
    unsigned int texturePaletteHandle_;
    unsigned int texturePaletteAllocatedSize_;

    void Initialize(unsigned int imageSpace, unsigned int paletteSpace);

    // reset the current state to the base state, functionally Reset() + Restore()
    // frees the entire arena
    void Reset();

    // 'commits' the current state stored in this instance to the NitroSDK
    // allocation code
    void Restore() const;
    // retrieves NitroSDK allocation state and stores it in this instance
    void Save();

    // Initializes a new arena whose base state is this arena's saved current state
    void CreateDescendant(TextureArena* to);

    unsigned int GetImageHandle() const { return textureImageHandle_; }
    unsigned int GetPaletteHandle() const { return texturePaletteHandle_; }
};