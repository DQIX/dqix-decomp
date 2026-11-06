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
#include "GeometryFifo.h"
#include "RenderConfig.h"

// Documentation on the various formats is available here:
// https://github.com/scurest/nsbmd_docs/blob/master/nsbmd_docs.txt

#define SIGNATURE_NSBMD 0x30444d42 // "BMD0"
#define SIGNATURE_NSBTX 0x30585442 // "BTX0"
#define SIGNATURE_NSBCA 0x30414342 // "BCA0"
#define SIGNATURE_NSBTP 0x30505442 // "BTP0"
#define SIGNATURE_NSBTA 0x30415442 // "BTA0"
#define SIGNATURE_NSBMA 0x30414d42 // "BMA0"
#define SIGNATURE_NSBVA 0x30415642 // "BVA0"

// usa: func_020b736c
// Gets the entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
void* Nitro3D_GetNameListEntry(const Nitro3DNameList* nameList, const char* name);

// usa: func_020b752c
// Gets the index of an entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
int Nitro3D_GetNameListIndex(const Nitro3DNameList* nameList, const char* name);

// usa: func_020b7694
// Returns a pointer to the first file within the file provided.
// In practice, this mainly seems to be used for getting the MDL
// out of an NSBMD, though I've also seen a use for getting a TEX
// out of an NSBTX, so I'm giving it the more generic name for now.
void* Nitro3D_GetFirstSubfile(Nitro3DFileHeader* nsbxx);

// usa: func_020b76a4
// Returns a pointer to a TEX0 file within the file provided.
// Assumes that the provided file either
//  - contains multiple files, of which the second is a TEX, or
//  - is an NSBTX containing only one file.
// From limited testing the first case is often satisfied
// by NSBMD files that contain one MDL and one TEX.
Nitro3DTextureSet* Nitro3D_GetTexturesFromFile(Nitro3DFileHeader* nsbxx);

// Various subfiles consist of multiple objects (e.g. MDL files
// comprise multiple models, JNT files contain multiple animations)
// and the subfile has the following format:
// uint32_t signature; // e.g. "MDL0"
// uint32_t filesize; 
// NameListU32 offsetsToObjectWithin;
// For example, this can be used to extract J.AC animations fron an nsbca file,
// M.AT animations from an nsbta file etc, or a Nitro3DModel from an
// nsbmd file
void* Nitro3D_GetInnerObjectByIndex(Nitro3DFileHeader* nsbxx, unsigned int idx);