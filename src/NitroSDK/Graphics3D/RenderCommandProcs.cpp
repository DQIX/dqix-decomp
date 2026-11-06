#include "NitroSDK/Graphics3D/Internal.h"

// Texture matrix generation procedures:
// In all instances, we build a texture matrix of the form 
//  ( a b 0 0 )  
//  ( c d 0 0 )                     ( a b )
//  ( 0 0 0 0 ), i.e. a linear term ( c d ) plus a translation by (e, f).
//  ( e f 0 1 )
// Usually we choose to do this by issuing the load 4x4 or multiply 4x4 commands,
// but one of the modes uses 4x3 instead. The matrix will combine rotation,
// translation and scaling based on what data is provided, but the interpretation
// of the data varies with the type - for example, rotation could be with respect
// to the middle of the texture or to a particular corner.
//
// Type 0: combine appropriate transformations in the order rotate -> translate -> scale.
// Rotation is aspect-ratio corrected. Scaling is centered at (0, 16h) i.e. the 
// bottom left corner, while rotation is centered at (8w, 8h) i.e. the center
// of the texture and goes counter-clockwise. The invididual matrices are
//  R = (   cos(u)    -h/w sin(u) ) with translation ( 8w(1 - sin(u) - cos(u)),
//      (  w/h sin(u)    cos(u)   )                    8h(1 + sin(u) - cos(u)) )
//
//  T = ( 1 0 ) with translation ( -16w t_x, 16h t_y )
//      ( 0 1 )
//
// S = ( s_x  0  ) with translation (0, 16h(1 - s_y))
//     (  0  s_y )
//
// Type 1: ignore rotation entirely and compose in order translate -> scale.
// Scaling is centered at the origin and translation is negative in both components.
// This implementation uses a 4x3 matrix. The individual matrices are
//  T = ( 1 0 ) with translation (-16w t_x, -16h t_y)
//      ( 0 1 )
// 
// S = ( s_x  0  ) with translation (0, 0)
//     (  0  s_y )
//
// Type 2: very weird and perhaps broken, combines appropriate transformations
// in the order translate -> rotate -> scale. Scaling is centered at (8w, 8h) i.e.
// in the middle of the texture. Rotation is aspect-ratio corrected and goes clockwise
//  but doesn't seem to have a consistent center? The individual matrices are
//  T = ( 1 0 ) with translation ( -16w t_x, 16h t_y )
//      ( 0 1 )
//
//  R = (   cos(u)     h/w sin(u) ) with translation ( 8w(1 - cos(u)) + 8h * sin(u),
//      ( -w/h sin(u)    cos(u)   )                    8h(1 - cos(u)) - 8w * sin(u) )
//
// S = ( s_x  0  ) with translation (8w(1 - s_x), 8h(1 - s_y))
//     (  0  s_y )
//
// Type 3: combine appropriate transformations in the order rotate -> translate -> scale.
// Rotation is aspect-ratio corrected and goes clockwise. Both scaling and rotation
// are centered at (0, 16h), i.e the bottom left corner. The individual matrices are
//  R = (   cos(u)     h/w sin(u) ) with translation (16w sin(u), 16h(1 - cos(u)))
//      ( -w/h sin(u)    cos(u)   )
// 
// T = ( 1 0 ) with translation ( -16w t_x, 16h t_y )
//     ( 0 1 )
//
// S = ( s_x  0  ) with translation (0, 16h(1 - s_y))
//     (  0  s_y )

void Nitro3Di_BoneMatrixDataSubmissionProcType0(BoneMatrixRenderData* renderData)
{
    if (!(renderData->flags & 4))
    {
        if (!(renderData->flags & 2))
        {
            // Combine rotation matrix with translation part
            Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)renderData->rotationMatrix.entries, 12);
        }
        else
        {
            Nitro3Di_SendFifoCommand(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate, 3);
        }
    }
    else if (!(renderData->flags & 2))
    {
        Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)renderData->rotationMatrix.entries, 9);
    }

    if (!(renderData->flags & 1))
    {
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v0, 3);
    }
}

void Nitro3Di_BoneMatrixScaleCalculationProcType0(BoneMatrixRenderData* renderData, Nitro3DBoneMatrix::Scaling* scaling, uint8_t* ip, int boneMatrixFlags)
{
    if (boneMatrixFlags & 4) // bone matrix has no scaling data
        renderData->flags |= 1;
    else
    {
        renderData->scale_v0.x = scaling->x;
        renderData->scale_v0.y = scaling->y;
        renderData->scale_v0.z = scaling->z;
    }

    renderData->flags |= 0x18;
}

void Nitro3Di_BoneMatrixDataSubmissionProcType1(BoneMatrixRenderData* renderData)
{
    bool needSubmitTranslateCommand = false;
    if (!(renderData->flags & 4))
        needSubmitTranslateCommand = true;

    if ((renderData->flags & 0x20) && !(renderData->flags & 0x08))
    {
        if (needSubmitTranslateCommand)
        {
            Nitro3Di_SendFifoCommand(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate, 3);
            needSubmitTranslateCommand = false;
        }
        // secondary scaling should happen before translation but after everything else
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v1, 3);
    }

    if (!(renderData->flags & 2)) // has rotation
    {
        if (needSubmitTranslateCommand)
        {
            // this 4x3 matrix represents rotating first, then translating
            Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)renderData->rotationMatrix.entries, 12);
            needSubmitTranslateCommand = false; // unused
        }
        else
            Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)renderData->rotationMatrix.entries, 9);
    }
    else if (needSubmitTranslateCommand)
    {
        Nitro3Di_SendFifoCommand(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate, 3);
        needSubmitTranslateCommand = false; // unused
    }

    if (!(renderData->flags & 1)) // has scale
    {
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v0, 3);
    }
}

void Nitro3Di_BoneMatrixScaleCalculationProcType1(BoneMatrixRenderData* renderData, Nitro3DBoneMatrix::Scaling* scaling, uint8_t* ip, int boneMatrixFlags)
{
    int thirdArg = ip[3];
    if (boneMatrixFlags & 4) // no scaling
    {
        renderData->flags |= 1;
        if (thirdArg & 2)
        {
            unsigned int thisBoneIdx = ip[1];
            g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[thisBoneIdx >> 5] |= (1 << (thisBoneIdx & 0x1f));
        }
    }
    else
    {
        renderData->scale_v0.x = scaling->x;
        renderData->scale_v0.y = scaling->y;
        renderData->scale_v0.z = scaling->z;
        if (thirdArg & 2)
        {
            unsigned int thisBoneIdx = ip[1];
            g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[thisBoneIdx >> 5] &= ~(1 << (thisBoneIdx & 0x1f));
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec1.x = scaling->x_v2;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec1.y = scaling->y_v2;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec1.z = scaling->z_v2;
        }
    }

    if (thirdArg & 1)
    {
        unsigned int parentBoneIdx = ip[2];
        renderData->flags |= 0x20;
        if (g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[parentBoneIdx >> 5] & (1 << (parentBoneIdx & 0x1f)))
        {
            renderData->flags |= 8;
        }
        else
        {
           renderData->scale_v1 = g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec1;
        }
    }

    renderData->flags |= 0x10;
}

static void CreateTextureMatrix_v0_RotateTranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationSine) >> 12;
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationCosine) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationSine) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationCosine) >> 12;
    
    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    // As a real number, this is
    // m_41 = 8w * s_x(1 - sin(u) - cos(u)) - 16w * s_x * t_x
    matrix->entries[12] = renderData->materialWidth * (renderData->extensionScaleX - (scaleXsin + scaleXcos)) * 8
        - renderData->materialWidth * (int)(((int64_t)renderData->extensionScaleX * (int64_t)renderData->translateX) >> 8);

    // As a real number this is
    // m_42 = 8h(2 + s_x(sin(u) - cos(u) - 1)) + 16h * s_y * t_y
    matrix->entries[13] = renderData->materialHeight * (scaleYsin - scaleYcos - renderData->extensionScaleY + (2 << 12)) * 8 + 
        + renderData->materialHeight * (int)(((int64_t)renderData->extensionScaleY * (int64_t)renderData->translateY) >> 8);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (scaleXsin * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v0_RotateTranslate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    matrix->entries[0] = renderData->rotationCosine;
    matrix->entries[5] = renderData->rotationCosine;
    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-renderData->rotationSine * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    fix32_t negsum = (renderData->rotationSine + renderData->rotationCosine);
    negsum = -negsum;
    matrix->entries[12] = renderData->materialWidth * (negsum + (1 << 12)) * 8
        - renderData->translateX * renderData->materialWidth * 16;

    matrix->entries[13] = renderData->materialHeight * (renderData->rotationSine - renderData->rotationCosine + (1 << 12)) * 8
        + renderData->translateY * renderData->materialHeight * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (renderData->rotationSine * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v0_TranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX;
    matrix->entries[5] = renderData->extensionScaleY;
    matrix->entries[1] = 0;

    matrix->entries[12] = renderData->materialWidth * -(fix32_t)(((int64_t)renderData->extensionScaleX * (int64_t)renderData->translateX) >> 8);

    matrix->entries[13] = 
        renderData->materialHeight * ((-2 * renderData->extensionScaleY) + (2 << 12)) * 8 +
        renderData->materialHeight * (fix32_t)(((int64_t)renderData->extensionScaleY * (int64_t)renderData->translateY) >> 8);

    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v0_Translate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[5] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[12] = -(renderData->translateX * renderData->materialWidth) * 16;
    matrix->entries[13] = (renderData->translateY * renderData->materialHeight) * 16;
    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v0_RotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;
    fix32_QueueComputeQuotient(matHeight, matWidth);

    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationSine) >> 12;
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationCosine) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationSine) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationCosine) >> 12;

    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;
    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-scaleYsin * aspectRatio) >> 12;
    
    fix32_QueueComputeQuotient(matWidth, matHeight);

    matrix->entries[12] = renderData->materialWidth * (renderData->extensionScaleX - (scaleXsin + scaleXcos)) * 8;
    matrix->entries[13] = renderData->materialHeight * ((scaleYsin - scaleYcos) - renderData->extensionScaleY + (2 << 12)) * 8;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (scaleXsin * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v0_Rotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;
    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine;
    matrix->entries[5] = renderData->rotationCosine;
    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-renderData->rotationSine * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    
    // stupid way to write forces correct assembly, as a real number this
    // is m_41 = 8w(1 - sin(u) - cos(u))
    fix32_t foo = renderData->materialWidth;
    fix32_t negsum = (renderData->rotationSine + renderData->rotationCosine);
    negsum = -negsum;
    negsum += (1 << 12);
    foo = foo * negsum * 8;
    matrix->entries[12] = foo;
    // as a real number this is m_42 = 8h(1 + sin(u) - cos(u))
    matrix->entries[13] = renderData->materialHeight * ((renderData->rotationSine - renderData->rotationCosine) + (1 << 12)) * 8;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (renderData->rotationSine * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v0_Scale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX;
    matrix->entries[5] = renderData->extensionScaleY;
    matrix->entries[1] = 0;
    
    matrix->entries[12] = 0;
    matrix->entries[13] = renderData->materialHeight * ((-2 * renderData->extensionScaleY) + (2 << 12)) * 8;

    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v0_NoExtensions(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[4] = 0;
    matrix->entries[5] = 1 << 12;
    matrix->entries[12] = 0;
    matrix->entries[13] = 0;
}

void Nitro3Di_MaterialTextureMatrixLoadProcType0(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x4 matrix;
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x4, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x4, GXFifoCommand_SetMatrixMode);
    
    fifoData.initialMatrixMode = 3; // texture matrix
    fifoData.finalMatrixMode = 2; // return to position+vector mode

    // initialize the 9+1 trivial entries
    fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[6] 
        = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[9]
        = fifoData.matrix.entries[10] = fifoData.matrix.entries[11] = fifoData.matrix.entries[14] = 0;
    fifoData.matrix.entries[15] = 1 << 12;

    static void (*matrixCompositionProcs[])(Matrix4x4*, MaterialRenderData*) =
    {
        &CreateTextureMatrix_v0_RotateTranslateScale,
        &CreateTextureMatrix_v0_RotateTranslate,
        &CreateTextureMatrix_v0_TranslateScale,
        &CreateTextureMatrix_v0_Translate,
        &CreateTextureMatrix_v0_RotateScale,
        &CreateTextureMatrix_v0_Rotate,
        &CreateTextureMatrix_v0_Scale,
        &CreateTextureMatrix_v0_NoExtensions
    };

    // Build the six nontrivial entries of the matrix by composing rotation,
    // translation and scale in that order (keep only the ones we have data for)
    matrixCompositionProcs[renderData->flags & 7](&fifoData.matrix, renderData);
    
    if (renderData->materialScaleX != 1 << 12)
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[1] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[1]) >> 12;
        fifoData.matrix.entries[12] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[12]) >> 12;
    }

    if (renderData->materialScaleY != 1 << 12)
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[5] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[5]) >> 12;
        fifoData.matrix.entries[13] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[13]) >> 12;
    }

    Nitro3Di_SendFifoCommand(fifoData.commands, (uint32_t*)&fifoData + 1, 18);
}

void Nitro3Di_BoneMatrixDataSubmissionProcType2(BoneMatrixRenderData* renderData)
{
    bool bVar2 = false;
    int flag10orflag8 = renderData->flags & 0x18;
    if (!flag10orflag8)
    {
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v2, 3);
    }
    if (!(renderData->flags & 4)) // has translation
    {
        if (flag10orflag8)
            bVar2 = true;
        else
        {
            uint32_t scaledTranslation[3];
            scaledTranslation[0] = ((int64_t)renderData->translate.x * renderData->scale_v1.x) >> 12;
            scaledTranslation[1] = ((int64_t)renderData->translate.y * renderData->scale_v1.y) >> 12;
            scaledTranslation[2] = ((int64_t)renderData->translate.z * renderData->scale_v1.z) >> 12;
            Nitro3Di_SendFifoCommand(GXFifoCommand_TranslateMatrix, scaledTranslation, 3);
        }
    }

    if (!(renderData->flags & 2)) // has rotation
    {
        if (bVar2) // do rotation then translation
            Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)renderData->rotationMatrix.entries, 12);
        else
            Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)renderData->rotationMatrix.entries, 9);
    }
    else if (bVar2)
    {
        Nitro3Di_SendFifoCommand(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate, 3);
    }

    if (!flag10orflag8)
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v1, 3);

    if (!(renderData->flags & 1)) // has scale
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v0, 3);
}

void Nitro3Di_BoneMatrixScaleCalculationProcType2(BoneMatrixRenderData* renderData, Nitro3DBoneMatrix::Scaling* scaling, uint8_t* ip, int boneMatrixFlags)
{
    unsigned int thisBoneIdx = ip[1];
    unsigned int parentBoneIdx = ip[2];
    if (boneMatrixFlags & 4) // no scaling
    {
        renderData->flags |= 1; // no scaling
        if (g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[parentBoneIdx >> 5] & (1 << (parentBoneIdx & 0x1f)))
        {
            g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[thisBoneIdx >> 5] |= (1 << (thisBoneIdx & 0x1f));
            renderData->flags |= 0x18;
        }
        else
        {
            Nitro3DSkinningScale* parent = &g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx];
            func_020ca408(parent, &g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx], sizeof(Nitro3DSkinningScale));
            // copy to scale2 and scale3
            func_020ca408(parent, &renderData->scale_v1, 2 * 3 * sizeof(fix32_t));
        }
    }
    else
    {
        renderData->scale_v0.x = scaling->x;
        renderData->scale_v0.y = scaling->y;
        renderData->scale_v0.z = scaling->z;
        if (g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[parentBoneIdx >> 5] & (1 << (parentBoneIdx & 0x1f)))
        {
            func_020ca408(scaling, &g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx], 2 * 3 * sizeof(fix32_t));
            g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[thisBoneIdx >> 5] &= ~(1 << (thisBoneIdx & 0x1f));
            renderData->flags |= 0x18;
        }
        else
        {
            g_nitro3DActiveRenderCommandHandler->boneMatrixBitfield[thisBoneIdx >> 5] &= ~(1 << (thisBoneIdx & 0x1f));
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec0.x = ((int64_t)scaling->x * (int64_t)g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec0.x) >> 12;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec0.y = ((int64_t)scaling->y * (int64_t)g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec0.y) >> 12;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec0.z = ((int64_t)scaling->z * (int64_t)g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec0.z) >> 12;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec1.x = ((int64_t)scaling->x_v2 * (int64_t)g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec1.x) >> 12;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec1.y = ((int64_t)scaling->y_v2 * (int64_t)g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec1.y) >> 12;
            g_nitro3DSkinningEquationBuffer.scaleVectors[thisBoneIdx].vec1.z = ((int64_t)scaling->z_v2 * (int64_t)g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx].vec1.z) >> 12;
            func_020ca408(&g_nitro3DSkinningEquationBuffer.scaleVectors[parentBoneIdx], &renderData->scale_v1, 2 * 3 * sizeof(fix32_t));
        }
    }
}

void Nitro3Di_MaterialTextureMatrixLoadProcType1(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x3 matrix; // local_38
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x3, GXFifoCommand_SetMatrixMode);
    fifoData.initialMatrixMode = 3; // switch to texture matrix mode
    fifoData.finalMatrixMode = 2; // back to position+vector

    fifoData.matrix.entries[1] = fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[5]
        = fifoData.matrix.entries[6] = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[11] = 0;
    
    // This version only does translation and scaling, translation is negated and
    // applies before scaling.
    if (renderData->flags & 4) // no translation data
    {
        fifoData.matrix.entries[9] = 0;
        fifoData.matrix.entries[10] = 0;
        if (renderData->flags & 1) // no scaling data
        {
            fifoData.matrix.entries[0] = 1 << 12;
            fifoData.matrix.entries[4] = 1 << 12;
        }
        else
        {
            fifoData.matrix.entries[0] = renderData->extensionScaleX;
            fifoData.matrix.entries[4] = renderData->extensionScaleY;
        }
    }
    else // has translation data
    {
        if (renderData->flags & 1) // no scaling data
        {
            fix32_t tx = -(renderData->translateX * 16);
            fifoData.matrix.entries[9] = tx * renderData->materialWidth;
            fix32_t ty = -(renderData->translateY * 16);
            fifoData.matrix.entries[10] = ty * renderData->materialHeight;
            fifoData.matrix.entries[0] = 1 << 12;
            fifoData.matrix.entries[4] = 1 << 12;
        }
        else
        {
            fifoData.matrix.entries[9] = renderData->materialWidth * -(fix32_t)(((int64_t)renderData->extensionScaleX * renderData->translateX) >> 8);
            fifoData.matrix.entries[10] = renderData->materialHeight * -(fix32_t)(((int64_t)renderData->extensionScaleY * renderData->translateY) >> 8);
            fifoData.matrix.entries[0] = renderData->extensionScaleX;
            fifoData.matrix.entries[4] = renderData->extensionScaleY;
        }
    }

    if (renderData->materialScaleX != (1 << 12))
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[9] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[9]) >> 12;
    }

    if (renderData->materialScaleY != (1 << 12))
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[10] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[10]) >> 12;
    }

    Nitro3Di_SendFifoCommand(fifoData.commands, (uint32_t*)&fifoData + 1, 14);
}

static void CreateTextureMatrix_v2_TranslateRotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationCosine) >> 12;
    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationSine) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationCosine) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationSine) >> 12;
    
    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    int shiftedTx = -renderData->materialWidth * (1 << 11) - renderData->translateX * renderData->materialWidth;
    int shiftedTy = -renderData->materialHeight * (1 << 11) + renderData->translateY * renderData->materialHeight;

    matrix->entries[12] = (fix32_t)((((int64_t)scaleXcos * shiftedTx) 
        - ((int64_t)scaleXsin * shiftedTy)) >> 8)
        + renderData->materialWidth * (8 << 12);

    matrix->entries[13] = (fix32_t)((((int64_t)scaleYsin * shiftedTx)
        + ((int64_t)scaleYcos * shiftedTy)) >> 8)
        + renderData->materialHeight * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-scaleXsin * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v2_TranslateRotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine;
    matrix->entries[5] = renderData->rotationCosine;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    // these hold (-w/2 - w * t_x) and (-h/2 + h * t_y) respectively
    fix32_t shiftedTx = -renderData->materialWidth * (1 << 11) - renderData->translateX * renderData->materialWidth;
    fix32_t shiftedTy = -renderData->materialHeight * (1 << 11) + renderData->translateY * renderData->materialHeight;
    
    matrix->entries[12] = (fix32_t)((((int64_t)renderData->rotationCosine * shiftedTx) 
        - ((int64_t)renderData->rotationSine * shiftedTy)) >> 8)
        + renderData->materialWidth * (8 << 12);
    matrix->entries[13] = (fix32_t)((((int64_t)renderData->rotationSine * shiftedTx) 
        + ((int64_t)renderData->rotationCosine * shiftedTy)) >> 8)
        + renderData->materialHeight * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v2_TranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX;
    matrix->entries[5] = renderData->extensionScaleY;

    matrix->entries[1] = 0;

    int tx = (-renderData->materialWidth * 0x800 - renderData->translateX * renderData->materialWidth);
    int ty = (-renderData->materialHeight * 0x800 + renderData->translateY * renderData->materialHeight);

    matrix->entries[12] = (fix32_t)(((int64_t)renderData->extensionScaleX * tx) >> 8) + renderData->materialWidth * (8 << 12);
    matrix->entries[13] = (fix32_t)(((int64_t)renderData->extensionScaleY * ty) >> 8) + renderData->materialHeight * (8 << 12);

    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v2_Translate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[5] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[12] = -renderData->translateX * renderData->materialWidth * 16;
    matrix->entries[13] = renderData->translateY * renderData->materialHeight * 16;
    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v2_RotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationCosine) >> 12;
    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX * (int64_t)renderData->rotationSine) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationCosine) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY * (int64_t)renderData->rotationSine) >> 12;
    
    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    int shiftedTx = renderData->materialWidth;
    int shiftedTy = renderData->materialHeight;
    shiftedTy = -shiftedTy * ((1 << 12) >> 1);
    shiftedTx = -shiftedTx * ((1 << 12) >> 1);

    matrix->entries[12] = (fix32_t)((((int64_t)scaleXcos * shiftedTx) 
        - ((int64_t)scaleXsin * shiftedTy)) >> 8)
        + renderData->materialWidth * (8 << 12);

    matrix->entries[13] = (fix32_t)((((int64_t)scaleYsin * shiftedTx)
        + ((int64_t)scaleYcos * shiftedTy)) >> 8)
        + renderData->materialHeight * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-scaleXsin * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v2_Rotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine;
    matrix->entries[5] = renderData->rotationCosine;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    fix32_t shiftedTx = renderData->materialWidth;
    fix32_t shiftedTy = renderData->materialHeight;
    shiftedTy = -shiftedTy * ((1 << 12) >> 1); // holds -height/2 as fix32
    shiftedTx = -shiftedTx * ((1 << 12) >> 1); // holds -width/2 as fix32

    matrix->entries[12] = (fix32_t)((((int64_t)renderData->rotationCosine * shiftedTx) 
        - ((int64_t)renderData->rotationSine * shiftedTy)) >> 8)
        + renderData->materialWidth * (8 << 12);
    matrix->entries[13] = (fix32_t)((((int64_t)renderData->rotationSine * shiftedTx) 
        + ((int64_t)renderData->rotationCosine * shiftedTy)) >> 8)
        + renderData->materialHeight * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v2_Scale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX;
    matrix->entries[5] = renderData->extensionScaleY;

    matrix->entries[1] = 0;

    matrix->entries[12] = ((1 << 12) - renderData->extensionScaleX) * renderData->materialWidth * 8;
    matrix->entries[13] = ((1 << 12) - renderData->extensionScaleY) * renderData->materialHeight * 8;

    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v2_NoExtensions(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[4] = 0;
    matrix->entries[5] = 1 << 12;
    matrix->entries[12] = 0;
    matrix->entries[13] = 0;
}

void Nitro3Di_MaterialTextureMatrixLoadProcType2(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x4 matrix;
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x4, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x4, GXFifoCommand_SetMatrixMode);

    fifoData.initialMatrixMode = 3; // load/adjust texture matrix
    fifoData.finalMatrixMode = 2; // return to position+vector mode after

    fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[6]
        = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[9]
        = fifoData.matrix.entries[10] = fifoData.matrix.entries[11] = fifoData.matrix.entries[14] = 0;
    fifoData.matrix.entries[15] = 1 << 12;

    static void (*matrixCompositionProcs[])(Matrix4x4*, MaterialRenderData*) =
    {
        &CreateTextureMatrix_v2_TranslateRotateScale,
        &CreateTextureMatrix_v2_TranslateRotate,
        &CreateTextureMatrix_v2_TranslateScale,
        &CreateTextureMatrix_v2_Translate,
        &CreateTextureMatrix_v2_RotateScale,
        &CreateTextureMatrix_v2_Rotate,
        &CreateTextureMatrix_v2_Scale,
        &CreateTextureMatrix_v2_NoExtensions
    };

    // apply whichever of translation, rotation, scale are available.
    // Note that this time, translation happens first. Also rotation is stupid,
    // the center of rotation is not (8w, 8h) like in v0.
    matrixCompositionProcs[renderData->flags & 7](&fifoData.matrix, renderData);

    if (renderData->materialScaleX != (1 << 12))
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[1] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[1]) >> 12;
        fifoData.matrix.entries[12] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[12]) >> 12;
    }

    if (renderData->materialScaleY != (1 << 12))
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[5] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[5]) >> 12;
        fifoData.matrix.entries[13] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[13]) >> 12;
    }

    Nitro3Di_SendFifoCommand(fifoData.commands, (uint32_t*)&fifoData + 1, 18);
}

static void CreateTextureMatrix_v3_RotateTranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    fix32_t cosine = renderData->rotationCosine;
    fix32_t sine = renderData->rotationSine;
    fix32_t scaleX = renderData->extensionScaleX;
    fix32_t scaleY = renderData->extensionScaleY; 

    fix32_t scaleXcos = ((int64_t)scaleX * cosine) >> 12;
    
    matrix->entries[0] = scaleXcos;

    fix32_t scaleXsin = ((int64_t)scaleX * sine) >> 12;
    fix32_t scaleYcos = ((int64_t)scaleY * cosine) >> 12;
    fix32_t scaleYsin = ((int64_t)scaleY * sine) >> 12;    

    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    
    int64_t txsin = (int64_t)renderData->translateX * renderData->rotationSine;
    int64_t tysin = (int64_t)renderData->translateY * renderData->rotationSine;
    int64_t txcos = (int64_t)renderData->translateX * renderData->rotationCosine;
    int64_t tycos = (int64_t)renderData->translateY * renderData->rotationCosine;
    
    int64_t rotatedty = (txsin - tycos) >> 12;
    fix32_t scaledrotatedty = (fix32_t)((rotatedty * renderData->extensionScaleY) >> 12);
    fix32_t m42prescaled = scaleYcos + scaledrotatedty - 0x1000;
    
    int64_t rotatedtx = (txcos + tysin) >> 12;

    fix32_t scaledrotatedtx = (rotatedtx * renderData->extensionScaleX) >> 12;
    fix32_t m41prescaled = scaleXsin - scaledrotatedtx;
    
    matrix->entries[12] = renderData->materialWidth * m41prescaled * 16;
    matrix->entries[13] = -renderData->materialHeight * m42prescaled * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-scaleXsin * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v3_RotateTranslate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    matrix->entries[0] = renderData->rotationCosine;
    matrix->entries[5] = renderData->rotationCosine;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine * aspectRatio) >> 12;
    fix32_QueueComputeQuotient(matWidth, matHeight);

    int64_t sine = renderData->rotationSine;
    int64_t cosine = renderData->rotationCosine;
    int64_t basetx = renderData->translateX;
    int64_t basety = renderData->translateY;
    
    int64_t xcos = basetx * cosine;
    int64_t ysin = basety * sine;    

    int32_t tx = (xcos + ysin) >> 12;

    int64_t ycos = basety * cosine;
    int64_t xsin = basetx * sine;
    
    int32_t ty = (xsin - ycos) >> 12;
    
    matrix->entries[12] = renderData->materialWidth * (renderData->rotationSine - tx) * 16;
    matrix->entries[13] = -renderData->materialHeight * (renderData->rotationCosine + ty - 0x1000) * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v3_TranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX;
    matrix->entries[5] = renderData->extensionScaleY;
    matrix->entries[1] = 0;

    
    fix32_t scaledTx = ((int64_t)renderData->translateX * renderData->extensionScaleX) >> 12;
    fix32_t scaledTy = ((int64_t)-renderData->translateY * renderData->extensionScaleY) >> 12;

    matrix->entries[12] = renderData->materialWidth * -scaledTx * 16;
    matrix->entries[13] = -renderData->materialHeight * (renderData->extensionScaleY + scaledTy - 0x1000) * 16;

    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v3_Translate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[5] = 1 << 12;
    matrix->entries[1] = 0;

    fix32_t tx = -renderData->translateX;
    fix32_t ty = -renderData->translateY;

    matrix->entries[12] = renderData->materialWidth * tx * 16;
    matrix->entries[13] = -renderData->materialHeight * ty * 16;
    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v3_RotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    fix32_t sine = renderData->rotationSine;
    fix32_t cosine = renderData->rotationCosine;

    fix32_t xsin = ((int64_t)renderData->extensionScaleX * sine) >> 12;    
    fix32_t xcos = ((int64_t)renderData->extensionScaleX * cosine) >> 12;
    
    fix32_t scaleY = renderData->extensionScaleY;

    fix32_t ycos = ((int64_t)scaleY * cosine) >> 12;

    matrix->entries[0] = xcos;
    matrix->entries[5] = ycos;

    fix32_t aspectRatio = fix32_GetDivisionResult();

    fix32_t ysine = ((int64_t)scaleY * sine) >> 12;
    matrix->entries[1] = (ysine * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    matrix->entries[12] = renderData->materialWidth * xsin * 16;
    matrix->entries[13] = -renderData->materialHeight * (ycos - 0x1000) * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-xsin * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v3_Rotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth << 12;
    fix32_t matHeight = renderData->materialHeight << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine;
    matrix->entries[5] = renderData->rotationCosine;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine * aspectRatio) >> 12;
    
    fix32_QueueComputeQuotient(matWidth, matHeight);

    matrix->entries[12] = renderData->materialWidth * renderData->rotationSine * 16;
    matrix->entries[13] = -renderData->materialHeight * (renderData->rotationCosine - (1 << 12)) * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine * invAspectRatio) >> 12;
}

static void CreateTextureMatrix_v3_Scale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX;
    matrix->entries[5] = renderData->extensionScaleY;
    matrix->entries[1] = 0;
    matrix->entries[12] = 0;
    matrix->entries[13] = -renderData->materialHeight * (renderData->extensionScaleY - (1 << 12)) * 16;
    matrix->entries[4] = 0;
}

static void CreateTextureMatrix_v3_NoExtensions(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[4] = 0;
    matrix->entries[5] = 1 << 12;
    matrix->entries[12] = 0;
    matrix->entries[13] = 0;
}

void Nitro3Di_MaterialTextureMatrixLoadProcType3(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x4 matrix;
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x4, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x4, GXFifoCommand_SetMatrixMode);

    fifoData.initialMatrixMode = 3; // load/adjust texture matrix
    fifoData.finalMatrixMode = 2; // return to position+vector mode after

    fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[6]
        = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[9]
        = fifoData.matrix.entries[10] = fifoData.matrix.entries[11] = fifoData.matrix.entries[14] = 0;
    fifoData.matrix.entries[15] = 1 << 12;

    if (renderData->flags & 1)
    {
        renderData->extensionScaleX = renderData->extensionScaleY = 1 << 12;
    }

    if (renderData->flags & 2)
    {
        renderData->rotationCosine = 1 << 12;
        renderData->rotationSine = 0;
    }

    if (renderData->flags & 4)
    {
        renderData->translateX = renderData->translateY = 0;
    }

    static void (*matrixCompositionProcs[])(Matrix4x4*, MaterialRenderData*) =
    {
        &CreateTextureMatrix_v3_RotateTranslateScale,
        &CreateTextureMatrix_v3_RotateTranslate,
        &CreateTextureMatrix_v3_TranslateScale,
        &CreateTextureMatrix_v3_Translate,
        &CreateTextureMatrix_v3_RotateScale,
        &CreateTextureMatrix_v3_Rotate,
        &CreateTextureMatrix_v3_Scale,
        &CreateTextureMatrix_v3_NoExtensions
    };

    matrixCompositionProcs[renderData->flags & 7](&fifoData.matrix, renderData);

    if (renderData->materialScaleX != (1 << 12))
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[1] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[1]) >> 12;
        fifoData.matrix.entries[12] = ((int64_t)renderData->materialScaleX * fifoData.matrix.entries[12]) >> 12;
    }

    if (renderData->materialScaleY != (1 << 12))
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[5] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[5]) >> 12;
        fifoData.matrix.entries[13] = ((int64_t)renderData->materialScaleY * fifoData.matrix.entries[13]) >> 12;
    }

    Nitro3Di_SendFifoCommand(fifoData.commands, (uint32_t*)&fifoData + 1, 18);
}