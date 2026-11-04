#pragma once

#include "Common.h"
#include "BoneMatrix.h"
#include "JAC.h"
#include "MAM.h"
#include "MAT.h"
#include "MPT.h"
#include "VAV.h"
#include "Model.h"
#include "Texture.h"
#include "Material.h"
#include "RenderCommands.h"

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

extern "C"
{
// usa: func_020b2e3c
int NSBXX_Tex_GetBlock1Length(Nitro3DTextureSet* tex);
// usa: func_020b2e50
int NSBXX_Tex_GetBlock2Length(Nitro3DTextureSet* tex);
// usa: func_020b2e64
void NSBXX_Tex_WriteImageVRAMOffsets(Nitro3DTextureSet* tex, int block1, int block2_3);
// usa: func_020b28e78
void NSBXX_Tex_LoadImageToVRAM(Nitro3DTextureSet* tex, bool needsMapping);
// usa: func_020b2f30
int NSBXX_Tex_GetBlock4Length(Nitro3DTextureSet* tex);
// usa: func_020b2f44
void NSBXX_Tex_WritePaletteVRAMOffset(Nitro3DTextureSet* tex, int offset);
// usa: func_020b2f4c
void NSBXX_Tex_LoadPaletteToVRAM(Nitro3DTextureSet* tex, bool needsMapping);

// usa: func_020b3184
bool NSBXX_AttachTextureImageToModel(Nitro3DModel* model, Nitro3DTextureSet* tex0);
// usa: func_020b3284
void NSBXX_DetachTextureImageFromModel(Nitro3DModel* model);
// usa: func_020b33f0
bool NSBXX_AttachTexturePaletteToModel(Nitro3DModel* model, Nitro3DTextureSet* tex0);
// usa: func_020b34f8
void NSBXX_DetachTexturePaletteFromModel(Nitro3DModel* model);
// usa: func_020b357c
int NSBXX_LinkTEX0ToMDL0(Nitro3DModelSet* mdl0, Nitro3DTextureSet* tex0);
// usa: func_020b362c
void NSBXX_UnlinkTEX0FromMDL0(Nitro3DModelSet* mdl0);

// usa: func_020b66f4
void NSBXX_Model_DrawShadow(Nitro3DModel* model, unsigned int arg_2, unsigned int arg_3, unsigned int arg_4);

// 
struct Nitro3DAnimation* NSBXX_Model_AllocateAnimationData(class AllocatorUnion* alloc, const void* rawAnim, Nitro3DModel* model);

// usa: func_020b6ebc
// applies to all materials of the model. If value == 0 then the bits specified
// by mask will be cleared, otherwise they will be set
void NSBXX_Model_SetAllMaterialFlags(Nitro3DModel* model, int value, unsigned int mask);

// usa: func_020b6f64
// applies to all materials of the model. If value == 0 then the bits specified
// by mask will be cleared, otherwise they will be set
void NSBXX_Model_AdjustPolygonAttrMask(Nitro3DModel* model, int setBits, unsigned int mask);

// usa: func_020b7184
// The alpha value can be between 0-31.
// (This function should become static later)
void NSBXX_Model_SetMaterialAlpha(Nitro3DModel* model, unsigned int materialIndex, int alpha);

// usa: func_020b71fc
int NSBXX_Model_GetMaterialPolygonID(Nitro3DModel* model, unsigned int materialIndex);

// usa: func_020b726c
// color to be specified with red in bits 0-4, green in bits 5-9, blue in bits 10-14
void NSBXX_Model_SetDiffuseReflectionColor(Nitro3DModel* model, int rgb);

// usa: func_020b72ac
// color to be specified with red in bits 0-4, green in bits 5-9, blue in bits 10-14
void NSBXX_Model_SetAmbientReflectionColor(Nitro3DModel* model, int col);

// usa: func_020b732c
// The alpha value can be between 0-31.
void NSBXX_Model_SetAlpha(Nitro3DModel* model, int alpha);

// usa: func_020b736c
// Gets the entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
void* NSBXXNameList_Search(const Nitro3DNameList* nameList, const char* name);

// usa: func_020b752c
// Gets the index of an entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
int NSBXXNameList_SearchIndex(const Nitro3DNameList* nameList, const char* name);

// usa: func_020b7694
// Returns a pointer to the first file within the file provided.
// In practice, this mainly seems to be used for getting the MDL
// out of an NSBMD, though I've also seen a use for getting a TEX
// out of an NSBTX, so I'm giving it the more generic name for now.
void* NSBXX_GetFirstSubfile(Nitro3DFileHeader* nsbxx);

// usa: func_020b76a4
// Returns a pointer to a TEX0 file within the file provided.
// Assumes that the provided file either
//  - contains multiple files, of which the second is a TEX, or
//  - is an NSBTX containing only one file.
// From limited testing the first case is often satisfied
// by NSBMD files that contain one MDL and one TEX.
Nitro3DTextureSet* NSBXX_GetTEXFile(Nitro3DFileHeader* nsbxx);

// Various subfiles consist of multiple objects (e.g. MDL files
// comprise multiple models, JNT files contain multiple animations)
// and the subfile has the following format:
// uint32_t signature; // e.g. "MDL0"
// uint32_t filesize; 
// NameListU32 offsetsToObjectWithin;
// For example, this can be used to extract J.AC animations fron an nsbca file,
// M.AT animations from an nsbta file etc, or an NSBXXInternalModel from an
// nsbmd file
void* NSBXX_GetObjectFromFirstSubfile(Nitro3DFileHeader* nsbxx, unsigned int idx);

// Might have other uses, but for now I've only seen it called on
// pattern animations.
const char* NSBXX_PatternAnimation_GetTextureName(Nitro3DMPT* anim, unsigned int idx);
const char* NSBXX_PatternAnimation_GetPaletteName(Nitro3DMPT* anim, unsigned int idx);
Nitro3DMPT::Track::Keyframe* NSBXX_PatternAnimation_GetKeyframe(Nitro3DMPT*, uint16_t track, uint16_t frameTime);
Nitro3DMPT::Track* NSBXX_PatternAnimation_GetTrack(Nitro3DMPT*, unsigned int track);
}