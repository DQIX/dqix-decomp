#pragma once

#include "std_library_functions.h"
#include <globaldefs.h>
#include "../Vector.h"

// Documentation on the various formats is available here:
// https://github.com/scurest/nsbmd_docs/blob/master/nsbmd_docs.txt
// I'm using NSBXX as a generic name for any of these file types

#define SIGNATURE_NSBMD 0x30444d42 // "BMD0"
#define SIGNATURE_NSBTX 0x30585442 // "BTX0"
#define SIGNATURE_NSBCA 0x30414342 // "BCA0"
#define SIGNATURE_NSBTP 0x30505442 // "BTP0"
#define SIGNATURE_NSBTA 0x30415442 // "BTA0"
#define SIGNATURE_NSBMA 0x30414d42 // "BMA0"
#define SIGNATURE_NSBVA 0x30415642 // "BVA0"

struct NSBXXContainer
{
    uint32_t signature_;
    uint16_t byteOrderMark_;
    uint16_t version_;
    uint32_t fileSize_;
    uint16_t headerSize_;
    uint16_t numSubfiles_;
};

struct NSBXXNameList
{
    uint8_t unknown_0_;
    uint8_t numEntries_;
    uint16_t totalSize_; // size in bytes of the entire list

    // subheader?
    uint16_t subheaderSize_;
    uint16_t offsetToDataStart_;

    struct SearchTreeEntry // https://github.com/scurest/nsbmd_docs/issues/2
    {
        uint8_t bitIndex_;
        uint8_t children_[2];
        uint8_t resourceIndex_;
    } treeRoot_8_;
    // Supposedly the structure continues as follows:
    // SearchTreeEntry otherEntries_[numEntries_];
    // <-- this + offsetToDataStart_ points to the following:
    // uint16_t elementStride_; // holds sizeof(T)
    // uint16_t dataSectionSize_; // includes elementStride and dataSectionSize, so is equal to numEntries * sizeof(T) + 4
    // T dataElements_[numEntries_];
    // Name names_[numEntries_]; // Name is an array of 16 chars

    // to do: clean up all of this mess, I don't want to change any existing
    // function until I know it won't affect any existing use

    template<class T>
    inline T* GetEntryByIndex(unsigned int n) const volatile
    {
        if (this != NULL && n < numEntries_)
        {
            intptr_t dataStart = (intptr_t)this + offsetToDataStart_;
            uint16_t stride = *(uint16_t*)dataStart;
            return (T*)(dataStart + 4 + stride * n);
        }
        return NULL;
    }

    template<class T>
    inline T* GetEntryByIndex_notVolatile(unsigned int n) const
    {
        if (this != NULL && n < numEntries_)
        {
            intptr_t dataStart = (intptr_t)this + offsetToDataStart_;
            uint16_t stride = *(uint16_t*)dataStart;
            return (T*)(dataStart + 4 + stride * n);
        }
        return NULL;
    }

    template<class T>
    inline T* GetEntryFromu32Offset(unsigned int n) const
    {
        uint32_t* pOffset = GetEntryByIndex_notVolatile<uint32_t>(n);
        if (pOffset != NULL)
            return (T*)((intptr_t)this + *pOffset);
        return NULL;
    }

    template<class T>
    inline T* GetEntryFromu32Offset_v2(unsigned int n) const
    {
        if (this != NULL)
        {
            uint32_t* pOffset = GetEntryByIndex_notVolatile<uint32_t>(n);
            if (pOffset != NULL)
                return (T*)((intptr_t)this + *pOffset);
        }
        return NULL;
    }

    inline const char* GetNameByIndexAndOffset(unsigned int n, unsigned int offsetInBuffer) const
    {
        if (this != NULL && n < numEntries_)
        {
            intptr_t dataStart = (intptr_t)this + offsetToDataStart_;
            intptr_t nameStart = dataStart + *(uint16_t*)(dataStart + 2);
            return (const char*)(nameStart + offsetInBuffer);
        }
        return NULL;
    }

    // 12 million helper functions that build up the 'get entry' function. By 
    // composing them we trick the compiler into still inlining them even in
    // regular -O2
    inline void* GetDataStart() const { return (void*)((intptr_t)this + offsetToDataStart_); }
    inline void* GetEntryv3Helper(unsigned int index, void* dataStart) const 
    {
        return (void*)((intptr_t)dataStart + 4 + *(uint16_t*)dataStart * index);
    }

    inline void* GetEntryv3(unsigned int index) const
    {
        return GetEntryv3Helper(index, GetDataStart());
    }

    inline void* GetEntryv3Safe(unsigned int index) const
    {
        return (this != NULL && index < numEntries_) ? GetEntryv3(index) : NULL;
    }

    inline void* GetEntryFromPtrOffset(void* pOffset) const
    {
        return (void*)((intptr_t)this + *(uint32_t*)pOffset);
    }

    inline void* GetEntryFromOffsetList_v3(unsigned int index) const
    {
        uint32_t* pOffset;
        return (this != NULL && (pOffset = (uint32_t*)GetEntryv3Safe(index))) ? 
            GetEntryFromPtrOffset(pOffset) : NULL;
    }
};

struct NSBXXInnerFileCommon {
    uint32_t signature;
    uint32_t fileSize;
    NSBXXNameList nameList;
};

struct NSBXXBoneMatrix
{
    // bit 0: set if *no* translation data
    // bit 1: set if *no* matrix-based rotation
    // bit 2: set if *no* scaling
    // bit 3: set if there *is* a pivot matrix
    // If there is a pivot matrix, then we also use:
    // bits 4-7: form
    // bit 8: negate 1
    // bit 9: negate c 
    // bit 10: negate d
    // bits 11-15: index of bone matrix on stack?
    uint16_t flags_;

    fix16_t m_11;

    struct Translation // present if flag bit 1 is zero. might be a Vector3i
    {
        fix32_t x;
        fix32_t y;
        fix32_t z;
    };

    // present if flag bit 3 is set.
    // The data (form, negate_1, negate_c, negate_d, a, b)
    // encode a PivotMatrix as follows: form is between 0 and 8
    // and encodes the position in a 3x3 matrix (column major, so e.g.
    // 3 means middle entry of top row) of an entry that is +1 or -1.
    // Which one it is depends on negate_1 (set: -1, clear: +1). The other
    // entries in its row and column are zero. The other four entries are
    // a, b, c and d in memory order, where a and b are specified here, and
    // c is either +b or -b (depending on negate_c) and d is either +a or -a
    // (depending on negate_d). Note the nsbmd docs have c paired with a and
    // d paired with b, but (at least in this codebase, and probably more
    // broadly considering the shape of rotation matrices) this is a typo.
    // Pictures of these matrices are available in the nsbmd docs.
    struct PivotMatrixData
    {
        fix16_t a;
        fix16_t b;
    };

    struct RotationMatrixData // present if flag 3 is clear and flag 1 is clear
    {
        fix16_t entries[8];
    };

    struct Scaling // present if flag bit 2 is zero
    {
        fix32_t x;
        fix32_t y;
        fix32_t z;

        fix32_t x_v2;
        fix32_t y_v2;
        fix32_t z_v2;
    };
};

struct NSBXXAnimationSignature
{
    // holds e.g. "J\0AC" or "M\0PT" but in practice is read
    // using a uint8_t and a uint16_t
    uint8_t signatureInitial_;
    uint8_t signatureNull_;
    uint16_t signatureEnd_;
};

struct NSBXXAnimationJAC
{
    NSBXXAnimationSignature signature_; // "J\0AC"
    uint16_t numFrames_;
    uint16_t numTracks_;
    uint32_t unk_8;
    uint32_t pivotDataOffset_;
    uint32_t basisMatricesOffset_;

    uint16_t trackOffsets_[1];

    struct Track
    {
        // bit 0: no channels at all
        // bits 1,2: no translation channels
        // bit 3: x-translation is constant
        // bit 4: y-translation is constant
        // bit 5: z-translation is constant
        // bits 6,7: no rotation channel
        // bit 8: rotation is constant
        // bits 9,10: no scale channels
        // bit 11: x-scale is constant
        // bit 12: y-scale is constant
        // bit 13: z-scale is constant
        // bits 14-23: ???, unused?
        // bits 24-31: index of bone matrix to target
        uint32_t flagsAndTargetBoneMatrix_;

        // channel data goes here...

        struct ChannelNonConst
        {
            // bits 0-15: start frame
            // bits 16-27: end frame
            // bits 28-29: 'width', determines type of data
            // bits 30-31: log(rate)
            uint32_t metadata_;
            uint32_t samplesOffset_;
        };
    };

    struct ScaleSample16
    {
        fix16_t primary_;
        fix16_t secondary_;
    };

    struct ScaleSample32
    {
        fix32_t primary_;
        fix32_t secondary_;
    };

    struct PivotMatrix
    {
        // bits 0-3: form (values 0 to 8)
        // bit 4: unit is -1
        // bit 5: c = -b instead of c = b
        // bit 6: d = -a instead of d = a
        int16_t flags;
        fix16_t a;
        fix16_t b;
    };

    struct BasisMatrix
    {
        int16_t data[5];
    };

    inline PivotMatrix* GetPivotMatrices() const
    {
        return (PivotMatrix*)((intptr_t)this + this->pivotDataOffset_);
    }

    inline BasisMatrix* GetBasisMatrices() const
    {
        return (BasisMatrix*)((intptr_t)this + this->basisMatricesOffset_);
    }
};

struct NSBXXAnimationMAM
{
    NSBXXAnimationSignature signature_;
    uint16_t unk_4;
    uint16_t unk_6;
    NSBXXNameList tracks_;

    // Each entry in the track describes how to vary the corresponding 
    // material property. The 32 bits are used as follows:
    // bits 0-15: offset, relative to this MAM, to array of samples
    // bits 16-28: index of last frame (i.e. number of frames - 1)
    // bit 29: is constant
    // bits 30-31: log_2(sample rate), only 0,1,2 supported
    // If bit 29 is set, then there is no sample array and instead bits 0-15
    // hold the constant value. Color samples (all except alpha) are 16-bit
    // with only 15 bits used, alpha samples are 8-bit with only 5 bits used.
    struct Track
    {
        uint32_t diffuse_;
        uint32_t ambient_;
        uint32_t reflection_;
        uint32_t emission_;
        uint32_t alpha_;
    };
};

struct NSBXXAnimationMAT
{
    NSBXXAnimationSignature signature_;
    uint16_t unk_4;
    uint16_t unk_6;
    NSBXXNameList tracks_;


    struct Track
    {
        uint32_t unk_0;
        uint32_t unk_4;
        uint32_t unk_8;
        uint32_t unk_c;
        uint32_t unk_10;
        uint32_t unk_14;
        uint32_t unk_18;
        uint32_t unk_1c;
        uint32_t unk_20;
        uint32_t unk_24;
    };
};

struct NSBXXAnimationMPT
{
    NSBXXAnimationSignature signature_;
    uint16_t numFrames_;
    uint8_t numTextureNames_;
    uint8_t numPaletteNames_;
    uint16_t textureNamesOffset_;
    uint16_t paletteNamesOffset_;

    NSBXXNameList tracks_;

    struct Track
    {
        uint16_t numKeyframes_;
        uint16_t unk_2; // I thought keyframes was u32 but it's loaded as u16
        int16_t maybeSpeed_4_; // I imagine this is a 1.3.12 fixed point, it took values like 0x0555 (1/3)
        uint16_t keyframeArrayOffset_;

        struct Keyframe
        {
            uint16_t frameTime_;
            uint8_t textureIdx_;
            uint8_t paletteIdx_;
        };
    };
};

// not well documented at all, and doesn't seem to be used in DQIX anywhere.
// signature is V.AV and controls visibility by interacting with render command 2.
struct NSBXXAnimationVAV
{
    char unk_0[6];
    unsigned short numConditions_;
    char unk_8[4];
    // to determine visibility, you generate a test index via
    // testIndex = (frame * numConditions) + conditionIndex
    // then look up bitfield[testIndex >> 5] & (1 << (testIndex & 0x1f)).
    // if it's nonzero, the model is visible
    uint32_t bitfield_[1];
};

struct ModelBoundingBox
{
    // all quantities are 16-bit fixed points.
    // scurest nsbmd docs say last 3 are xMax etc but that's not how
    // they're used in functions here
    int16_t xMin_;
    int16_t yMin_;
    int16_t zMin_;
    int16_t xSize_;
    int16_t ySize_;
    int16_t zSize_;
};

struct NSBXXMaterial
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

struct NSBXXModelMaterialData
{
    // offset to a NameList of MaterialPairing structs
    uint16_t texturePairingsOffset_;
    // offset to a NameList of MaterialPairing structs
    uint16_t palettePairingsOffset_;
    // entries are uint32_t's giving offsets to NSBXXMaterial structs
    NSBXXNameList materialOffsetList_;

    inline struct NSBXXMaterial* GetMaterialByIndex(unsigned int n) const
    {
        if (this != NULL)
        {
            uint32_t* materialOffset = materialOffsetList_.GetEntryByIndex<uint32_t>(n);
            
            if (materialOffset != NULL)
            {
                return (NSBXXMaterial*)((intptr_t)this + *materialOffset);
            }
        }
        return NULL;
    }
};

// Texture image pairing or texture palette pairing.
// No information about the image/palette is stored in this struct, instead
// you use the name corresponding to this entry in the relevant NameList
// pointed to by the offsets in NSBXXModelMaterialData.
struct NSBXXMaterialPairing
{
    // offset to an array of uint8_t giving indices of Materials that use this
    // image / palette. Offsets to these materials are stored in a NameList,
    // and the uint8_t values are indices in that list.
    uint16_t offsetToIndexArray_;
    uint8_t arraySize_;
    uint8_t flags_; // bit 0 = has TEX0 data bound/linked to it
};

struct NSBXXMesh
{
    uint16_t unk_0;
    uint16_t size_;
    uint32_t unk_4;
    uint32_t gpuCommandsOffset_;
    uint32_t gpuCommandsLength_; // length in bytes

    inline const uint32_t* GetGPUCommands() const
    {
        return (uint32_t*)((intptr_t)this + gpuCommandsOffset_);
    }
};

struct NSBXXInvBindMatrix
{
    Matrix4x3 mat4x3;
    Matrix3x3 mat3x3;
};

struct NSBXXInternalModel
{
    uint32_t filesize_;
    uint32_t renderCommandsOffset_;
    uint32_t materialsOffset_;
    uint32_t meshesOffset_; // offset to a NameList of NSBXXMesh
    uint32_t inverseBindsOffset_;
    uint8_t unk_14;
    // 0, 1 or 2, determines the behavior of applying scaling
    // to bones. 0 = simplest, only have one scaling operation followed
    // by rotation and translation.
    // In modes 1 and 2, each bone also provides a scale factor to be
    // used by its children.
    // Might have something to do with 'segment scale compensate'
    // behaviour? https://download.autodesk.com/us/maya/2011help/nodes/joint.html
    uint8_t boneScalingMode_;
    uint8_t materialCallbackType_;
    uint8_t numBoneMatrices_;
    uint8_t numMaterials_;
    uint8_t numMeshes_;
    uint8_t unk_1a[2];
    fix32_t upScale_; // 32-bit fixed point
    fix32_t downScale_; // 32-bit fixed point
    uint16_t numVertices_;
    uint16_t numPolygons_;
    uint16_t numTriangles_;
    uint16_t numQuads_;
    ModelBoundingBox bounds_;
    int32_t maybeScale_;
    char unk_3c[4];
    NSBXXNameList boneList_;

    // using this seems to screw up register assignment, but it's here in case
    // we can make it work later somehow
    inline NSBXXModelMaterialData* GetMaterialData() const
    {
        if (this != NULL && materialsOffset_ != 0)
            return (NSBXXModelMaterialData*)((intptr_t)this + materialsOffset_);
        else
            return NULL;
    }

    inline NSBXXNameList* GetMeshListUnsafe() const
    {
        return (NSBXXNameList*)((intptr_t)this + meshesOffset_);
    }

    inline NSBXXNameList* GetMeshListSafe() const
    {
        return (this != NULL && meshesOffset_ != 0) ? GetMeshListUnsafe() : NULL;
    }

    inline NSBXXNameList* GetMeshList() const
    {
        if (this != NULL && meshesOffset_ != 0)
            return (NSBXXNameList*)((intptr_t)this + meshesOffset_);
        return NULL;
    }

    inline NSBXXMesh* GetMesh(unsigned int n) const
    {
        NSBXXNameList* meshList;
        if (this != NULL && meshesOffset_ != 0)
            meshList = (NSBXXNameList*)((intptr_t)this + meshesOffset_);
        else
            meshList = NULL;

        if (meshList != NULL)
        {
            return meshList->GetEntryFromu32Offset<NSBXXMesh>(n);
        }
        return NULL;
    }
};

struct NSBXXMdl
{
    uint32_t signature_;
    uint32_t unknown_4;
    NSBXXNameList nameList_;

    NSBXXInternalModel* GetInternalModelByIndex(unsigned int n) const
    {
        NSBXXInternalModel* internalModel;
        if (this != NULL)
        {
            uint32_t* pModelOffset = nameList_.GetEntryByIndex<uint32_t>(n);

            if (pModelOffset != NULL)
            {
                internalModel = (NSBXXInternalModel*)((intptr_t)this + *pModelOffset);
                return internalModel;
            }
        }
        return NULL;
    }
};

struct NSBXXTex
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
    NSBXXNameList textureList_;

    inline NSBXXNameList* GetPaletteList() const
    {
        if (this != NULL && paletteListOffset_ != 0)
            return (NSBXXNameList*)((intptr_t)this + paletteListOffset_);
        else
            return NULL;
    }
};

struct NSBXXTexTexture
{
    uint32_t paramTEXIMAGE_PARAMS_;
    uint32_t unk_4;
};

struct NSBXXTexPalette
{
    uint16_t offsetWithinBlock4_; // need to left-shift by 3
    // if bit 0 is set, the offset is right shifted by 1 before passing to
    // PLTT_BASE, i.e. PLTT_BASE is expecting offset/16 instead of offset/8.
    // This happens when texture format != 2
    uint16_t unk_2;
};

extern "C"
{
// usa: func_020b2e3c
int NSBXX_Tex_GetBlock1Length(NSBXXTex* tex);
// usa: func_020b2e50
int NSBXX_Tex_GetBlock2Length(NSBXXTex* tex);
// usa: func_020b2e64
void NSBXX_Tex_WriteImageVRAMOffsets(NSBXXTex* tex, int block1, int block2_3);
// usa: func_020b28e78
void NSBXX_Tex_LoadImageToVRAM(NSBXXTex* tex, bool needsMapping);
// usa: func_020b2f30
int NSBXX_Tex_GetBlock4Length(NSBXXTex* tex);
// usa: func_020b2f44
void NSBXX_Tex_WritePaletteVRAMOffset(NSBXXTex* tex, int offset);
// usa: func_020b2f4c
void NSBXX_Tex_LoadPaletteToVRAM(NSBXXTex* tex, bool needsMapping);

// usa: func_020b3184
bool NSBXX_AttachTextureImageToModel(NSBXXInternalModel* model, NSBXXTex* tex0);
// usa: func_020b3284
void NSBXX_DetachTextureImageFromModel(NSBXXInternalModel* model);
// usa: func_020b33f0
bool NSBXX_AttachTexturePaletteToModel(NSBXXInternalModel* model, NSBXXTex* tex0);
// usa: func_020b34f8
void NSBXX_DetachTexturePaletteFromModel(NSBXXInternalModel* model);
// usa: func_020b357c
int NSBXX_LinkTEX0ToMDL0(NSBXXMdl* mdl0, NSBXXTex* tex0);
// usa: func_020b362c
void NSBXX_UnlinkTEX0FromMDL0(NSBXXMdl* mdl0);

// usa: func_020b66f4
void NSBXX_Model_DrawShadow(NSBXXInternalModel* model, unsigned int arg_2, unsigned int arg_3, unsigned int arg_4);

// 
struct AnimationData* NSBXX_Model_AllocateAnimationData(class AllocatorUnion* alloc, const void* rawAnim, NSBXXInternalModel* model);

// usa: func_020b6ebc
// applies to all materials of the model. If value == 0 then the bits specified
// by mask will be cleared, otherwise they will be set
void NSBXX_Model_SetAllMaterialFlags(NSBXXInternalModel* model, int value, unsigned int mask);

// usa: func_020b6f64
// applies to all materials of the model. If value == 0 then the bits specified
// by mask will be cleared, otherwise they will be set
void NSBXX_Model_AdjustPolygonAttrMask(NSBXXInternalModel* model, bool setBits, unsigned int mask);

// usa: func_020b7184
// The alpha value can be between 0-31.
// (This function should become static later)
void NSBXX_Model_SetMaterialAlpha(NSBXXInternalModel* model, unsigned int materialIndex, int alpha);

// usa: func_020b71fc
int NSBXX_Model_GetMaterialPolygonID(NSBXXInternalModel* model, unsigned int materialIndex);

// usa: func_020b726c
// color to be specified with red in bits 0-4, green in bits 5-9, blue in bits 10-14
void NSBXX_Model_SetDiffuseReflectionColor(NSBXXInternalModel* model, int rgb);

// usa: func_020b72ac
// color to be specified with red in bits 0-4, green in bits 5-9, blue in bits 10-14
void NSBXX_Model_SetAmbientReflectionColor(NSBXXInternalModel* model, int col);

// usa: func_020b732c
// The alpha value can be between 0-31.
void NSBXX_Model_SetAlpha(NSBXXInternalModel* model, int alpha);

// usa: func_020b736c
// Gets the entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
void* NSBXXNameList_Search(NSBXXNameList* nameList, const char* name);

// usa: func_020b752c
// Gets the index of an entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
int NSBXXNameList_SearchIndex(NSBXXNameList* nameList, const char* name);

// usa: func_020b7694
// Returns a pointer to the first file within the file provided.
// In practice, this mainly seems to be used for getting the MDL
// out of an NSBMD, though I've also seen a use for getting a TEX
// out of an NSBTX, so I'm giving it the more generic name for now.
void* NSBXX_GetFirstSubfile(NSBXXContainer* nsbxx);

// usa: func_020b76a4
// Returns a pointer to a TEX0 file within the file provided.
// Assumes that the provided file either
//  - contains multiple files, of which the second is a TEX, or
//  - is an NSBTX containing only one file.
// From limited testing the first case is often satisfied
// by NSBMD files that contain one MDL and one TEX.
NSBXXTex* NSBXX_GetTEXFile(NSBXXContainer* nsbxx);

// Various subfiles consist of multiple objects (e.g. MDL files
// comprise multiple models, JNT files contain multiple animations)
// and the subfile has the following format:
// uint32_t signature; // e.g. "MDL0"
// uint32_t filesize; 
// NameListU32 offsetsToObjectWithin;
// For example, this can be used to extract J.AC animations fron an nsbca file,
// M.AT animations from an nsbta file etc, or an NSBXXInternalModel from an
// nsbmd file
void* NSBXX_GetObjectFromFirstSubfile(NSBXXContainer* nsbxx, unsigned int idx);

// Might have other uses, but for now I've only seen it called on
// pattern animations.
const char* NSBXX_PatternAnimation_GetTextureName(NSBXXAnimationMPT* anim, unsigned int idx);
const char* NSBXX_PatternAnimation_GetPaletteName(NSBXXAnimationMPT* anim, unsigned int idx);
NSBXXAnimationMPT::Track::Keyframe* NSBXX_PatternAnimation_GetKeyframe(NSBXXAnimationMPT*, uint16_t track, uint16_t frameTime);
NSBXXAnimationMPT::Track* NSBXX_PatternAnimation_GetTrack(NSBXXAnimationMPT*, unsigned int track);
}