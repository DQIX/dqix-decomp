#pragma once

#include "Model.h"
#include "Texture.h"

struct Nitro3DMaterial
{
    uint16_t unk_0;
    uint16_t size_; // in bytes
    uint32_t paramDIF_AMB_;
    uint32_t paramSPE_EMI_;
    uint32_t paramPOLYGON_ATTR_;
    uint32_t maskPOLYGON_ATTR_;
    uint32_t paramTEXIMAGE_PARAMS_;

    uint32_t unk_18;
    uint16_t texturePaletteVRAMOffset_;
    // bit 0: has any of type 1,2,3 extension data
    // bits 1-3: does *not* have type 1, 2, 3 extension data
    // bit 4: ???
    // bit 5: if set, the alpha gets overwritten to 0, what?
    // bits 6,7,8: which parts of paramDIF_AMB_ to use
    // (6 = use diffuse color, 7 = use ambient color, 8 = vertex color flag)
    // if a bit is set, the corresponding value will be taken from here, otherwise
    // the value specified in the RenderConfig will be used
    // bits 9,10,11: which parts of paramSPE_EMI_ to use 
    // bit 12: ???
    // bit 13: has bit-13 extension data (comes after 1,2,3 if present)
    uint16_t flags_;
    uint16_t width_;
    uint16_t height_;
    fix32_t xScale_; // to be filled when linked to actual texture data
    fix32_t yScale_; // to be filled when linked to actual texture data

    // Usually the size is 0x2c, but it can be larger and this seems
    // to be related to the bits at 0x1e. If bit 0 is set, then some of the
    // next three substructs are included, sequentially, depending whether
    // bits 1, 2 and 3 are *clear* respectively (i.e. if bit 1 is clear,
    // then substruct #1 is present)

    // Represents a scaling centred at (0, height) i.e. bottom left
    struct ExtensionData_Bit1
    {
        fix32_t scaleX_;
        fix32_t scaleY_;
    };

    // Represents a rotation around the centre of the texture.
    // Instead of specifying the angle we specify its sine & cosine
    struct ExtensionData_Bit2
    {
        fix16_t sine_;
        fix16_t cosine_;
    };

    // Represents a translation by (width * x, height * y) texels
    struct ExtensionData_Bit3
    {
        uint32_t translateX_;
        uint32_t translateY_;
    };
    
    struct ExtensionData_Bit13
    {
        Matrix4x4 matrix_;
    };
};

struct Nitro3DModelMaterials
{
    // offset to a NameList of MaterialPairing structs
    uint16_t texturePairingsOffset_;
    // offset to a NameList of MaterialPairing structs
    uint16_t palettePairingsOffset_;
    // entries are uint32_t's giving offsets to NSBXXMaterial structs
    Nitro3DNameList materialOffsetList_;

    inline struct Nitro3DMaterial* GetMaterialByIndex(unsigned int n) const
    {
        if (this != NULL)
        {
            uint32_t* materialOffset = (uint32_t*)materialOffsetList_.GetEntryByIndex<uint32_t>(n);
            if (materialOffset != NULL)
            {
                return (Nitro3DMaterial*)((intptr_t)this + *materialOffset);
            }
        }
        return NULL;
    }

    // Texture image pairing or texture palette pairing.
    // No information about the image/palette is stored in this struct, instead
    // you use the name corresponding to this entry in the relevant NameList
    // pointed to by the offsets in NSBXXModelMaterialData.
    struct Pair
    {
        // offset to an array of uint8_t giving indices of Materials that use this
        // image / palette. Offsets to these materials are stored in a NameList,
        // and the uint8_t values are indices in that list.
        uint16_t offsetToIndexArray_;
        uint8_t arraySize_;
        uint8_t flags_; // bit 0 = has TEX0 data bound/linked to it
    };
};

