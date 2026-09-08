#pragma once

// This header is for use only by the 2 (3) RenderCommands.cpp source files.
// Once command 9 is decompiled we can merge the files into one and paste
// this in instead

#include "NSBXX.h"
#include "../../System/Graphics.h"
#include "GeometryFifo.h"
#include "RenderCommands.h"
#include "RenderConfig.h"
#include "Animation.h"
#include <asmhacks.h>

#pragma optimize_for_size off

#if defined(jpn)
#define data_020e9240 data_020e934c
#define data_020e9260 data_020e936c
#define data_020e9284 data_020e9390
#define data_020f1ce0 data_020f1e4c
#define data_020f1cec data_020f1e58
#define data_020f1cf8 data_020f1e64
#define data_020f1d08 data_020f1e74
#define data_020f1d78 data_020f1ee4
#define data_020f1dc0 data_020f1f2c
#define data_020f1e08 data_020f1f74
#define data_0210a274 data_02109f2c
#define data_0210a278 data_02109f30
#define data_0210b078 data_0210ad30

#define func_020c1868 func_020c3334
#define func_020c223c func_020c3d08
#define func_020c2c5c func_020c4728
#define func_020c2cf0 func_020c47bc
#define func_020c2eb8 func_020c4984
#define func_020c2f18 func_020c49e4
#define func_020c54fc func_020c6fc8
#define func_020ca3ec func_020cbeb8
#define func_020ca408 func_020cbed4
#define func_020ca430 func_020cbefc
#define func_020ca458 func_020cbf24
#define func_020ca7d0 func_020cc29c
#endif

// holds values:
// { 0, 0x00007fff, 0x7fff0000, 0x7fff7fff, 
//   0x00008000, 0x0000ffff, 0x7fff8000, 0x7fffffff }
// i.e. data_020e9240[idx] sets bits
//    0-14 if (idx & 1)
//    16-30 if (idx & 2)
//    15 if (idx & 4)
// For DIF_AMB these are masks corresponding to diffuse reflection color,
// ambient reflection color and (set vertex color yes/no)
// For SPE_EMI these are masks corresponding to specular reflection color,
// emission color and (enable specular reflection shininess table yes/no)
extern unsigned int const data_020e9240[];

// pivot matrix a/b/c/d position lookup
// both arrays are identical
extern struct {
    uint8_t a, b, c, d;
} const data_020e9260[], data_020e9284[];

// Holds { func_020b9a2c, func_020b9b30, func_020ba390 }.
// Called by command 6 to populate scaling data for the bone matrix
extern void (*data_020f1cec[])(BoneMatrixRenderData*, NSBXXBoneMatrix::Scaling* boneMatrixScaleData, uint8_t* ip, int boneMatrixFlags);
// Holds { func_020b99b0, func_020b9a6c, func_020ba264 }
// Called by command 6 to apply bone matrix transformations via the GXFifo
extern void (*data_020f1ce0[])(BoneMatrixRenderData*);
// Holds { func_020ba11c func_020ba5ac, func_020bac74, func_020bb29c }
extern void (*data_020f1cf8[])(MaterialRenderData*);

extern struct Struct_020f1d08
{
    // used for render command 13
    unsigned int commandTexImageParams_0_; // holds 0x2A = GXFifoCommand_SetTexImageParams
    unsigned int texImageParamsArg_4_;
    // used for render command 12
    unsigned int commandTexImageParams_8_; // holds 0x2a = GXFifoCommand_SetTexImageParams
    unsigned int texImageParamsArg_c_;

    // only one of these is not null
    void (*materialBindFunctions[4])(RenderCommandHandler*, int modifier, NSBXXMaterial*, unsigned int idx);
    // only one of these is not null
    void (*meshDrawFunctions[4])(RenderCommandHandler*, int modifier, NSBXXMesh*, unsigned int idx);

    Matrix4x4 command13Matrix4x4_;
} data_020f1d08;

// a blob of GXFIFO data used by command 7. The entries are
// - combined command (12, 10, 17, 1b) = (pop matrix, matrix mode, load 3x4, scale)
// - 1 (parameter for pop matrix)
// - 2 (parameter for matrix mode: position + vector)
// - nine entries for the identity matrix
// - 3 entries that make up the translation component of the matrix. Initially
//   zero but can be modified
// - 3 parameters for the scaling command
struct Struct_020f1d78 {
    // is always (12, 10, 17, 1b) = (pop matrix, matrix mode, load 4x3, scale)
    // which can be merged with subsequent data as a single command to the GXFIFO.
    uint32_t commandsCombined;
    // always 1
    uint32_t popMatrixParameter;
    // always 2 (position + vector?)
    uint32_t matrixModeParameter;
    // this is always the 3x3 identity, but combined with translation gives a 4x3 matrix
    Matrix4x3 rotationTranslation;

    Vector3fix scaling;
} extern data_020f1d78, data_020f1dc0;

// render command lookup
extern void (*data_020f1e08[])(RenderCommandHandler*, int);

extern struct RenderCommandHandler* data_0210a274;
// sizeof == 0x38
MaterialRenderData extern data_0210a278[64];

struct Struct_0210b078
{
    Vector3fix vec0_;
    BoneMatrixRenderData::Scale vec1_;
} extern data_0210b078[64];

struct Struct_0210b678
{
    Matrix4x4 mat4x4;
    Matrix3x3 mat3x3;
} extern data_0210b678[];

extern "C"
{  
#if true
    // expand a 3x4 matrix (param 1) into a 4x4 matrix (param 2)
    void func_020c1868(const void*, void*);
    // multiply 4x4 matrices, store in param 3.
    // set up so that you can safely have param_3 = param_1 or param_2
    // The arrangement is such that param_3 represents the transformation
    // (do param_1) then (do param_2). In row major representation this is
    // param_1 * param_2, in col major it's param_2 * param_1.
    void func_020c223c(const void*, const void*, void*);
    // get length of vector3
    fix32_t func_020c2eb8(const void*);
    // fix32_t division: retrieve result from hardware divider
    fix32_t func_020c2c5c();
    // fix32_t division: set up hardware divider
    void func_020c2cf0(fix32_t a, fix32_t b);
    // normalize vector3 (param 1) into vector3 (param 2)
    void func_020c2f18(const void*, void*);
#endif
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