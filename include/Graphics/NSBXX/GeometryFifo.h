#pragma once

#include "std_library_functions.h"
#include "../Vector.h"
#include "Memory/AllocatorUnion.h"

// Note: the convention seems to be that matrices act on *row vectors*
// by right multiplication, rather than the more common setting of
// acting on column vectors by left multiplication. In particular, we agree
// with typical DirectX conventions instead of OpenGL / math.
// Matrices are stored in row major order, i.e. each row of the matrix
// is contiguous in memory. 
// GBATEK documentation is consistent with our choice, but the NSBMD
// docs are *not*. We choose to go with GBATEK specifically because certain
// SDK functions make more sense this way, e.g. matrix multiplication functions
// compute arg1 * arg2 and store it in arg3, but only if you view matrices
// as row major.
// As a result of this choice,
//   - A*B represents the transformation obtained by doing A first, then B
//   - Affine transformations have rightmost column (0, 0, 0, 1)^T and can
//     thus be represented concisely as a 4x3 matrix
//   - A translation matrix by (u, v, w) has bottom row equal to (u, v, w, 1)
enum GXFifoCommand
{
    // 1 parameter: 0-3 to specify mode.
    // 0 = projection, 1 = position, 2 = position+vector (?), 3 = texture
    GXFifoCommand_SetMatrixMode = 0x10,
    // 0 parameters
    GXFifoCommand_PushMatrix = 0x11,
    // 1 parameter: number N to reduce the stack pointer by.
    // Loads the matrix at the new stack top into the current matrix.
    // Usually N = 1, but can have -30 <= N <= 31.
    GXFifoCommand_PopMatrix = 0x12,
    // 1 parameter: position N to store the current matrix to.
    // To go to top of matrix stack, use PushMatrix instead
    GXFifoCommand_StoreMatrix = 0x13,
    // 1 parameter: position N to get the matrix from. This matrix
    // will be loaded into the current matrix.
    GXFifoCommand_GetMatrix = 0x14,
    // 0 parameters. Modifies the current matrix
    GXFifoCommand_LoadMatIdentity = 0x15,
    // 16 parameters, one for each entry (row-major order)
    // Modifies the current matrix
    GXFifoCommand_LoadMat4x4 = 0x16,
    // 12 parameters, one for each entry (row-major order)
    // Modifies the current matrix
    GXFifoCommand_LoadMat4x3 = 0x17,
    // 16 parameters, one for each entry (row-major order)
    // of a matrix M.
    // The current matrix C is transformed into M * C, i.e. the
    // parameter represents the first transformation to take place
    GXFifoCommand_MultiplyMat4x4 = 0x18,
    // 12 parameters, one for each entry (row-major order)
    // of a matrix M
    // The matrix is expanded to be 4x4 by adding (0, 0, 0, 1)^T
    // as a rightmost column, then the current matrix C
    // is transformed into M * C, i.e. the parameter represents
    // the first transformation to take place
    GXFifoCommand_MultiplyMat4x3 = 0x19,
    // 9 parameters, one for each entry (row-major order)
    // of a matrix M
    // The matrix is expanded to be 4x4 by adding a bottom row
    // and rightmost column, all of whose entries are 0 except
    // the bottom right entry which is 1. Then the current matrix
    // C is transformed into M * C, i.e. the parameter represents
    // the first transformation to take place
    GXFifoCommand_MultiplyMat3x3 = 0x1a,
    // 3 parameters (x, y, z). The current matrix C is 
    // transformed into ScalingMatrix(x, y, z) * C, i.e. the
    // scaling is the first transformation to take place.
    GXFifoCommand_ScaleMatrix = 0x1b,
    // 3 parameters (x, y, z). The current matrix C is
    // transformed into TranslationMatrix(x, y, z) * C i.e. the
    // translation is the first transformation to take place.
    GXFifoCommand_TranslateMatrix = 0x1c,

    // 1 parameter which packs: S (horizontal, x) coordinate
    // in bits 0-15, as a 1.11.4 fixed point number, T (vertical, y)
    // coordinate in bits 16-31, also as a 1.11.4 fixed point
    GXFifoCommand_SetTextureCoords = 0x22,

    // 1 parameter packing various quantities - see GBATEK
    // though notably bits 16-20 encode the alpha value (0 to 31)
    GXFifoCommand_SetPolygonAttr = 0x29,
    // 1 parameter which packs: ((VRAM offset of texture) >> 3) in
    // bits 0-15, repeat in S/T direction in bits 16/17, flip in
    // S/T direction in bits 18/19, log_2(S-size / 8) in bits 20-22,
    // log_2(T-size / 8) in bits 23-25, texture format in bits 26-28,
    // (make color 0 transparent yes/no) in bit 29.
    // See GBATEK for a list of texture formats
    GXFifoCommand_SetTexImageParams = 0x2a,
    // 1 parameter specifying either the palette base address divided by 8
    // (for texture format 2) or divided by 16 (all other formats)
    GXFifoCommand_SetTexturePaletteBase = 0x2b,


    // 1 parameter which packs: diffuse reflection color in bits
    // 0-14, set vertex color yes/no in bit 15, ambient reflection
    // color in bits 16-30
    GXFifoCommand_DiffuseAmbientReflect = 0x30,
    // 1 parameter which packs: specular reflection color in bits
    // 0-14, use specular reflection shininess table yes/no in bit 15,
    // emission color in bits 16-30
    GXFifoCommand_SpecularReflectEmit = 0x31,
    // 1 parameter which packs: x component in bits 0-9 (one bit for
    // sign, 9 bits for fractional part), y component in bits 10-19,
    // z component in bits 20-29 and light index in bits 30-31
    GXFifoCommand_SetLightVector = 0x32,
    // 1 parameter which packs: color in bits 0-14, light number in
    // bits 30-31
    GXFifoCommand_SetLightColor = 0x33,
    // 32 parameters viewed as 128 8-bit parameters (bits 0-7, then
    // bits 8-15, then bits 16-23, then bits 24-31). Each parameter
    // is the fractional part of a number between 0 and 1, i.e. a 
    // uint8_t equal to n is encoding the real number (n/256)
    GXFifoCommand_SetShininessTable = 0x34,

    // 1 parameter which packs: min x (left) in bits 0-7, min y (top)
    // in bits 8-15, max x (right) in bits 16-23, max y (bottom) in
    // bits 24-31
    GXFifoCommand_SetViewport = 0x60,

};

#define COMBINE_GXFIFO_COMMANDS2(a, b) ((int)(a) | ((int)(b) << 8))
#define COMBINE_GXFIFO_COMMANDS3(a, b, c) ((int)(a) | ((int)(b) << 8) | ((int)(c) << 16))
#define COMBINE_GXFIFO_COMMANDS4(a, b, c, d) ((int)(a) | ((int)(b) << 8) | ((int)(c) << 16) | ((int)(d) << 24))

extern "C"
{
    void SendQueuedDataToGeometryFifo();

    void SendRawDataToGeometryFifo(const uint32_t* data, unsigned int numBytes);
    // In practice you just send the command followed by the params, so this is
    // often used to submit multiple commands by just passing
    // (first word, ptr to second word, number total words - 1)
    void SubmitCommandToGeometryFifo(int command, const uint32_t* params, unsigned int numParams);
}

struct ModelRenderContext;
struct AnimationData;
struct NSBXXInternalModel;

// these don't really fit here but are a bit too random to have their own TU...

void GetCurrentPositionAndDirectionMatrices(Matrix4x3* position, Matrix3x3* direction);
bool GetModelBonePositionAndDirectionMatrices(ModelRenderContext* context, Matrix4x3* outPos, Matrix3x3* outDir, unsigned int boneIndex);
void Finish3DRendering();
// returns 0 if point is visible and -1 if not
int ConvertWorldToScreenCoordinates(const Vector3fix* world, int* outX, int* outY);