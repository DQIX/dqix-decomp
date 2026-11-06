#pragma once

// for internal use only, don't expose in main Graphics3D.h header.
// None of these things should be global, the layout is just too cursed to avoid it

#include "Common.h"
#include "Animation.h"

typedef void (*PFNInitializeModelAnimation)(Nitro3DAnimation*, void*, Nitro3DModel*);
typedef void (*PFNProcessAnimation)(void*, Nitro3DAnimation*, int);

typedef bool (*PFNProcessMaterialAnimations)(MaterialRenderData*, Nitro3DAnimation*, unsigned int);
typedef bool (*PFNProcessJointAnimations)(BoneMatrixRenderData*, Nitro3DAnimation*, unsigned int);
typedef bool (*PFNProcessVisibilityAnimations)(int*, Nitro3DAnimation*, unsigned int);

extern PFNProcessAnimation g_pfnNitro3DProcessVAV, g_pfnNitro3DProcessJAC, 
    g_pfnNitro3DProcessMAM, g_pfnNitro3DProcessMAT, g_pfnNitro3DProcessMPT;

extern PFNProcessMaterialAnimations g_pfnNitro3DProcessMaterialAnimations;
extern PFNProcessJointAnimations g_pfnNitro3DProcessJointAnimations;
extern PFNProcessVisibilityAnimations g_pfnNitro3DProcessVisibilityAnimations;

void Nitro3Di_InitAnimationFromJAC(Nitro3DAnimation* anim, void* pvJAC, Nitro3DModel* model);
void Nitro3Di_InitAnimationFromMAM(Nitro3DAnimation* anim, void* pvMAM, Nitro3DModel* model);
void Nitro3Di_InitAnimationFromMAT(Nitro3DAnimation* anim, void* pvMAT, Nitro3DModel* model);
void Nitro3Di_InitAnimationFromMPT(Nitro3DAnimation* anim, void* pvMPT, Nitro3DModel* model);
void Nitro3Di_InitAnimationFromVAV(Nitro3DAnimation* anim, void* pvVAV, Nitro3DModel* model);

void Nitro3Di_ProcessAnimationJAC(void* data, Nitro3DAnimation* anim, int arg);
void Nitro3Di_ProcessAnimationMAM(void* data, Nitro3DAnimation* anim, int arg);
void Nitro3Di_ProcessAnimationMAT(void* data, Nitro3DAnimation* anim, int arg);
void Nitro3Di_ProcessAnimationMPT(void* data, Nitro3DAnimation* anim, int arg);
void Nitro3Di_ProcessAnimationVAV(void* data, Nitro3DAnimation* anim, int arg);

bool Nitro3Di_DefaultProcessMaterialAnimations(MaterialRenderData* material, Nitro3DAnimation* anim, unsigned int matIdx);
bool Nitro3Di_DefaultProcessJointAnimations(BoneMatrixRenderData* bone, Nitro3DAnimation* anim, unsigned int boneIdx);
bool Nitro3Di_DefaultProcessVisibilityAnimations(int* output, Nitro3DAnimation* anim, unsigned int boneIdx);