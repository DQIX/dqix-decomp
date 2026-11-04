#pragma once

#include "Common.h"

// represents a TEX0 subfile in an NSBMD / NSBTX
struct Nitro3DTextureSet
{
    uint32_t signature;
    uint32_t maybeTotalSize_4_;
    // this is zero in the file but gets overwritten.
    // it actually contains a few things: low 16 bits are the offset
    // divided by eight. Then the next 15 bits encode the size divided by
    // sixteen, and the top bit is some bool.
    uint32_t block1VRAMLoadOffset_;
    uint16_t block1NumEightBytes_; // need to << 3 to get block size
    uint16_t textureListOffset_;
    uint16_t maybeBlock1Flags_10_; // bit 0: loaded to vram
    char padding_12[2];
    uint32_t block1Offset_;
    // see the block 1 analogue for a description
    uint32_t block2Or3VRAMLoadOffset_;
    uint16_t block2NumEightBytes_; // block 3 has as many 4-byte values, so << 3 and << 2 respectively
    uint16_t unk_1e;
    uint16_t maybeBlock23Flags_20_; // bit 0: loaded to vram
    char padding_22[2];
    uint32_t block2Offset_;
    uint32_t block3Offset_;
    // see the block 1 analogue for a description
    uint32_t block4VRAMLoadOffset_;
    uint16_t block4NumEightBytes_;
    uint16_t maybeBlock4Flags_32_; // bit 0: loaded to vram
    uint16_t paletteListOffset_; 
    uint16_t unk_36;
    uint32_t block4Offset_;

    // The documentation would suggest this is at a dynamic offset specified
    // by textureListOffset_, but some functions treat it as being in this fixed
    // location.
    Nitro3DNameList textureList_;

    inline Nitro3DNameList* GetPaletteList() const
    {
        if (this != NULL && paletteListOffset_ != 0)
            return (Nitro3DNameList*)((intptr_t)this + paletteListOffset_);
        else
            return NULL;
    }

    struct ImageData
    {
        uint32_t paramTEXIMAGE_PARAMS_;
        uint32_t unk_4;
    };

    struct PaletteData
    {
        uint16_t offsetWithinBlock4_; // need to left-shift by 3
        // if bit 0 is set, the offset is right shifted by 1 before passing to
        // PLTT_BASE, i.e. PLTT_BASE is expecting offset/16 instead of offset/8.
        // This happens when texture format != 2
        uint16_t unk_2;
    };
};