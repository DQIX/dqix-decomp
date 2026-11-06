#pragma once

// for internal use only, don't expose in main Graphics3D.h header.

#include "Graphics3D.h"
#include "../Graphics.h"
#include "GeometryFifo.h"
#include "RenderCommands.h"
#include "RenderConfig.h"
#include "Animation.h"
#include <asmhacks.h>

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

#if defined(jpn)
#define func_020c5188 func_020c6c54
#define func_020c51a4 func_020c6c70
#define func_020c51c0 func_020c6c8c
#define func_020c54fc func_020c6fc8
#define func_020c552c func_020c6ff8
#define func_020ca3ec func_020cbeb8
#define func_020ca408 func_020cbed4
#define func_020ca430 func_020cbefc
#define func_020ca458 func_020cbf24
#define func_020ca7d0 func_020cc29c
#endif


extern void (*g_nitro3DSubmitBoneMatrixProcs[])(BoneMatrixRenderData*);
extern void (*g_nitro3DPopulateBoneMatrixScalingProcs[])(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling* boneMatrixScaleData, uint8_t* ip, int boneMatrixFlags);
extern void (*g_nitro3DLoadMaterialTextureMatrixProcs[])(MaterialRenderData*);

extern Nitro3DRenderCommandHandler* g_nitro3DActiveRenderCommandHandler;
struct Nitro3DSkinningScale
{
    BoneMatrixRenderData::Scale vec0;
    BoneMatrixRenderData::Scale vec1;
};
struct Nitro3DSSkinningInvBindMatrix
{
    Matrix4x4 mat4x4;
    Matrix3x3 mat3x3;
};

struct Nitro3DSkinningEquationBuffer
{
    Nitro3DSkinningScale scaleVectors[64];
    Nitro3DSSkinningInvBindMatrix matrices[64];
} extern g_nitro3DSkinningEquationBuffer;

extern "C"
{  
    // load the specified matrix into the current matrix (3x4).
    void func_020c5188(const fix32_t*);
    // multiply the current matrix by the specified one (3x4), i.e carry
    // out GXFIFO operation 0x19
    void func_020c51a4(const fix32_t*);
    // multiply by the specified 3x3 matrix (instruction 0x1a)
    void func_020c51c0(const fix32_t*);
    // try to get the clip matrix into the pointer specified (4x4 fix32_t matrix)
    // returns -1 if still waiting, 0 if done
    int func_020c54fc(void*);
    // try to get the result matrix (current directional vector matrix)
    // returns -1 if still waiting, 0 if done
    int func_020c552c(void*);
    // memset via u32 values
    void func_020ca3ec(int value, void* dst, unsigned int len);
    // memcpy via u32 values
    void func_020ca408(const void* src, void* dst, unsigned int numBytes);
    // repeatedly write u32s to dst (useful for hardware registers)
    void func_020ca430(const void* src, volatile void* dst, unsigned int numBytes);
    // aligned 32-byte-looped memset
    void func_020ca458(int value, void* dst, unsigned int len);
    // zero-initialize matrix
    void func_020ca7d0(void*);
}

const char* Nitro3Di_GetPatternAnimationTextureName(Nitro3DMPT* anim, unsigned int idx);
const char* Nitro3Di_GetPatternAnimationPaletteName(Nitro3DMPT* anim, unsigned int idx);
Nitro3DMPT::Track::Keyframe* Nitro3Di_GetPatternAnimationKeyframe(Nitro3DMPT*, uint16_t track, uint16_t frameTime);
Nitro3DMPT::Track* Nitro3Di_GetPatternAnimationTrack(Nitro3DMPT*, unsigned int track);