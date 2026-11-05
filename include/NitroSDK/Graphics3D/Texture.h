#pragma once

#include "Common.h"

// represents a TEX0 subfile in an NSBMD / NSBTX
struct Nitro3DTextureSet
{
    uint32_t signature;
    uint32_t sizeBytes;
    // this is zero in the file but gets overwritten.
    // it actually contains a few things: low 16 bits are the offset
    // divided by eight. Then the next 15 bits encode the size divided by
    // sixteen, and the top bit is some bool.
    uint32_t block1VRAMLoadOffset;
    uint16_t block1NumEightBytes; // need to << 3 to get block size
    uint16_t textureListOffset;
    uint16_t maybeBlock1Flags_10; // bit 0: loaded to vram
    char pad_12[2];
    uint32_t block1Offset;
    // see the block 1 analogue for a description
    uint32_t block2Or3VRAMLoadOffset;
    uint16_t block2NumEightBytes; // block 3 has as many 4-byte values, so << 3 and << 2 respectively
    char unk_1e[2];
    uint16_t maybeBlock23Flags_20; // bit 0: loaded to vram
    char pad_22[2];
    uint32_t block2Offset;
    uint32_t block3Offset;
    // see the block 1 analogue for a description
    uint32_t block4VRAMLoadOffset;
    uint16_t block4NumEightBytes;
    uint16_t maybeBlock4Flags_32; // bit 0: loaded to vram
    uint16_t paletteListOffset; 
    char unk_36[2];
    uint32_t block4Offset;

    // The documentation would suggest this is at a dynamic offset specified
    // by textureListOffset_, but some functions treat it as being in this fixed
    // location.
    Nitro3DNameList textureList;

    inline Nitro3DNameList* GetPaletteList() const
    {
        if (this != NULL && paletteListOffset != 0)
            return (Nitro3DNameList*)((intptr_t)this + paletteListOffset);
        else
            return NULL;
    }

    struct ImageData
    {
        uint32_t paramTEXIMAGE_PARAMS;
        // bits 0-10: width
        // bits 11-21: height
        uint32_t dimensions;
    };

    struct PaletteData
    {
        uint16_t offsetWithinBlock4; // need to left-shift by 3
        // if bit 0 is set, the offset is right shifted by 1 before passing to
        // PLTT_BASE, i.e. PLTT_BASE is expecting offset/16 instead of offset/8.
        // This happens when texture format != 2
        uint16_t unknown_2;
    };
};

// usa: func_020b2e3c
int Nitro3D_GetTextureBlock1Length(const Nitro3DTextureSet* tex);
// usa: func_020b2e50
int Nitro3D_GetTextureBlock2Length(const Nitro3DTextureSet* tex);
// usa: func_020b2e64
void Nitro3D_StoreTextureImageVRAMOffsets(Nitro3DTextureSet* tex, int block1, int block2_3);
// usa: func_020b28e78
void Nitro3D_LoadTextureImageToVRAM(Nitro3DTextureSet* tex, bool needsMapping);
// usa: func_020b2f30
int Nitro3D_GetTextureBlock4Length(const Nitro3DTextureSet* tex);
// usa: func_020b2f44
void Nitro3D_StoreTexturePaletteVRAMOffsets(Nitro3DTextureSet* tex, int offset);
// usa: func_020b2f4c
void Nitro3D_LoadTexturePaletteToVRAM(Nitro3DTextureSet* tex, bool needsMapping);

// usa: func_020b3184
bool Nitro3D_AttachTextureImage(Nitro3DModel* model, Nitro3DTextureSet* tex0);
// usa: func_020b3284
void Nitro3D_DetachTextureImage(Nitro3DModel* model);
// usa: func_020b33f0
bool Nitro3D_AttachTexturePalette(Nitro3DModel* model, Nitro3DTextureSet* tex0);
// usa: func_020b34f8
void Nitro3D_DetachTexturePalette(Nitro3DModel* model);
// usa: func_020b357c
int Nitro3D_LinkTexturesToModels(Nitro3DModelSet* mdl0, Nitro3DTextureSet* tex0);
// usa: func_020b362c
void Nitro3D_UnlinkTexturesFromModels(Nitro3DModelSet* mdl0);