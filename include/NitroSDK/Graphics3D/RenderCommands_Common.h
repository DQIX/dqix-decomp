#pragma once

// This header is for use only by the 2 (3) RenderCommands.cpp source files.
// Once command 9 is decompiled we can merge the files into one and paste
// this in instead

#include "Graphics3D.h"
#include "../Graphics.h"
#include "GeometryFifo.h"
#include "RenderCommands.h"
#include "RenderConfig.h"
#include "Animation.h"
#include <asmhacks.h>

#if defined(jpn)
#define data_020e9284 data_020e9390
#define data_020f1ce0 data_020f1e4c
#define data_020f1cec data_020f1e58
#define data_020f1cf8 data_020f1e64

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

// Holds { func_020b99b0, func_020b9a6c, func_020ba264 }
// Called by command 6 to apply bone matrix transformations via the GXFifo
extern void (*g_nitro2DSubmitBoneMatrixProcs[])(BoneMatrixRenderData*);
// Holds { func_020b9a2c, func_020b9b30, func_020ba390 }.
// Called by command 6 to populate scaling data for the bone matrix
extern void (*g_nitro2DPopulateBoneMatrixScalingProcs[])(BoneMatrixRenderData*, Nitro3DBoneMatrix::Scaling* boneMatrixScaleData, uint8_t* ip, int boneMatrixFlags);
// Holds { func_020ba11c func_020ba5ac, func_020bac74, func_020bb29c }
extern void (*g_nitro2DLoadMaterialTextureMatrixProcs[])(MaterialRenderData*);

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