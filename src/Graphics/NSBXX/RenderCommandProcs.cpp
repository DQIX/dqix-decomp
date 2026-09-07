#include "Graphics/NSBXX/RenderCommands.h"
#include "Graphics/NSBXX/RenderCommands_Common.h"
#include "Graphics/NSBXX/GeometryFifo.h"

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

#if defined(jpn)
#define data_020f1e88 data_020f1ff4
#define data_020f1ea8 data_020f2014
#define data_020f1ec8 data_020f2034
#endif

#pragma optimize_for_size off

extern void (*data_020f1e88[8])(Matrix4x4*, MaterialRenderData*);
extern void (*data_020f1ea8[8])(Matrix4x4*, MaterialRenderData*);
extern void (*data_020f1ec8[8])(Matrix4x4*, MaterialRenderData*);

void BoneMatrixDataSubmissionProc_Type0(BoneMatrixRenderData* renderData)
{
    if (!(renderData->flags_ & 4))
    {
        if (!(renderData->flags_ & 2))
        {
            // Combine rotation matrix with translation part
            SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)renderData->rotationMatrix_.entries, 12);
        }
        else
        {
            SubmitCommandToGeometryFifo(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate_, 3);
        }
    }
    else if (!(renderData->flags_ & 2))
    {
        SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)renderData->rotationMatrix_.entries, 9);
    }

    if (!(renderData->flags_ & 1))
    {
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v0_, 3);
    }
}

void BoneMatrixScaleCalculationProc_Type0(BoneMatrixRenderData* renderData, NSBXXBoneMatrix::Scaling* scaling, uint8_t* ip, int boneMatrixFlags)
{
    if (boneMatrixFlags & 4) // bone matrix has no scaling data
        renderData->flags_ |= 1;
    else
    {
        renderData->scale_v0_.x = scaling->x;
        renderData->scale_v0_.y = scaling->y;
        renderData->scale_v0_.z = scaling->z;
    }

    renderData->flags_ |= 0x18;
}

void BoneMatrixDataSubmissionProc_Type1(BoneMatrixRenderData* renderData)
{
    bool needSubmitTranslateCommand = false;
    if (!(renderData->flags_ & 4))
        needSubmitTranslateCommand = true;

    if ((renderData->flags_ & 0x20) && !(renderData->flags_ & 0x08))
    {
        if (needSubmitTranslateCommand)
        {
            SubmitCommandToGeometryFifo(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate_, 3);
            needSubmitTranslateCommand = false;
        }
        // secondary scaling should happen before translation but after everything else
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v1_, 3);
    }

    if (!(renderData->flags_ & 2)) // has rotation
    {
        if (needSubmitTranslateCommand)
        {
            // this 4x3 matrix represents rotating first, then translating
            SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)renderData->rotationMatrix_.entries, 12);
            needSubmitTranslateCommand = false; // unused
        }
        else
            SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)renderData->rotationMatrix_.entries, 9);
    }
    else if (needSubmitTranslateCommand)
    {
        SubmitCommandToGeometryFifo(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate_, 3);
        needSubmitTranslateCommand = false; // unused
    }

    if (!(renderData->flags_ & 1)) // has scale
    {
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v0_, 3);
    }
}

void BoneMatrixScaleCalculationProc_Type1(BoneMatrixRenderData* renderData, NSBXXBoneMatrix::Scaling* scaling, uint8_t* ip, int boneMatrixFlags)
{
    int thirdArg = ip[3];
    if (boneMatrixFlags & 4) // no scaling
    {
        renderData->flags_ |= 1;
        if (thirdArg & 2)
        {
            unsigned int thisBoneIdx = ip[1];
            data_0210a274->boneMatrixBitfield_[thisBoneIdx >> 5] |= (1 << (thisBoneIdx & 0x1f));
        }
    }
    else
    {
        renderData->scale_v0_.x = scaling->x;
        renderData->scale_v0_.y = scaling->y;
        renderData->scale_v0_.z = scaling->z;
        if (thirdArg & 2)
        {
            unsigned int thisBoneIdx = ip[1];
            data_0210a274->boneMatrixBitfield_[thisBoneIdx >> 5] &= ~(1 << (thisBoneIdx & 0x1f));
            data_0210b078[thisBoneIdx].vec1_.x = scaling->x_v2;
            data_0210b078[thisBoneIdx].vec1_.y = scaling->y_v2;
            data_0210b078[thisBoneIdx].vec1_.z = scaling->z_v2;
        }
    }

    if (thirdArg & 1)
    {
        unsigned int parentBoneIdx = ip[2];
        renderData->flags_ |= 0x20;
        if (data_0210a274->boneMatrixBitfield_[parentBoneIdx >> 5] & (1 << (parentBoneIdx & 0x1f)))
        {
            renderData->flags_ |= 8;
        }
        else
        {
            renderData->scale_v1_ = data_0210b078[parentBoneIdx].vec1_;
        }
    }

    renderData->flags_ |= 0x10;
}

extern "C" void CreateTextureMatrix_v0_RotateTranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationSine_) >> 12;
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationCosine_) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationSine_) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationCosine_) >> 12;
    
    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    // As a real number, this is
    // m_41 = 8w * s_x(1 - sin(u) - cos(u)) - 16w * s_x * t_x
    matrix->entries[12] = renderData->materialWidth_ * (renderData->extensionScaleX_ - (scaleXsin + scaleXcos)) * 8
        - renderData->materialWidth_ * (int)(((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->translateX_) >> 8);

    // As a real number this is
    // m_42 = 8h(2 + s_x(sin(u) - cos(u) - 1)) + 16h * s_y * t_y
    matrix->entries[13] = renderData->materialHeight_ * (scaleYsin - scaleYcos - renderData->extensionScaleY_ + (2 << 12)) * 8 + 
        + renderData->materialHeight_ * (int)(((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->translateY_) >> 8);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (scaleXsin * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v0_RotateTranslate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    matrix->entries[0] = renderData->rotationCosine_;
    matrix->entries[5] = renderData->rotationCosine_;
    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-renderData->rotationSine_ * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    fix32_t negsum = (renderData->rotationSine_ + renderData->rotationCosine_);
    negsum = -negsum;
    matrix->entries[12] = renderData->materialWidth_ * (negsum + (1 << 12)) * 8
        - renderData->translateX_ * renderData->materialWidth_ * 16;

    matrix->entries[13] = renderData->materialHeight_ * (renderData->rotationSine_ - renderData->rotationCosine_ + (1 << 12)) * 8
        + renderData->translateY_ * renderData->materialHeight_ * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (renderData->rotationSine_ * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v0_TranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX_;
    matrix->entries[5] = renderData->extensionScaleY_;
    matrix->entries[1] = 0;

    matrix->entries[12] = renderData->materialWidth_ * -(fix32_t)(((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->translateX_) >> 8);

    matrix->entries[13] = 
        renderData->materialHeight_ * ((-2 * renderData->extensionScaleY_) + (2 << 12)) * 8 +
        renderData->materialHeight_ * (fix32_t)(((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->translateY_) >> 8);

    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v0_Translate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[5] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[12] = -(renderData->translateX_ * renderData->materialWidth_) * 16;
    matrix->entries[13] = (renderData->translateY_ * renderData->materialHeight_) * 16;
    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v0_RotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;
    fix32_QueueComputeQuotient(matHeight, matWidth);

    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationSine_) >> 12;
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationCosine_) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationSine_) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationCosine_) >> 12;

    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;
    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-scaleYsin * aspectRatio) >> 12;
    
    fix32_QueueComputeQuotient(matWidth, matHeight);

    matrix->entries[12] = renderData->materialWidth_ * (renderData->extensionScaleX_ - (scaleXsin + scaleXcos)) * 8;
    matrix->entries[13] = renderData->materialHeight_ * ((scaleYsin - scaleYcos) - renderData->extensionScaleY_ + (2 << 12)) * 8;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (scaleXsin * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v0_Rotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;
    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine_;
    matrix->entries[5] = renderData->rotationCosine_;
    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (-renderData->rotationSine_ * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    
    // stupid way to write forces correct assembly, as a real number this
    // is m_41 = 8w(1 - sin(u) - cos(u))
    fix32_t foo = renderData->materialWidth_;
    fix32_t negsum = -(renderData->rotationSine_ + renderData->rotationCosine_);
    negsum += (1 << 12);
    foo = foo * negsum * 8;
    matrix->entries[12] = foo;
    // as a real number this is m_42 = 8h(1 + sin(u) - cos(u))
    matrix->entries[13] = renderData->materialHeight_ * ((renderData->rotationSine_ - renderData->rotationCosine_) + (1 << 12)) * 8;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (renderData->rotationSine_ * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v0_Scale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX_;
    matrix->entries[5] = renderData->extensionScaleY_;
    matrix->entries[1] = 0;
    
    matrix->entries[12] = 0;
    matrix->entries[13] = renderData->materialHeight_ * ((-2 * renderData->extensionScaleY_) + (2 << 12)) * 8;

    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v0_NoExtensions(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[4] = 0;
    matrix->entries[5] = 1 << 12;
    matrix->entries[12] = 0;
    matrix->entries[13] = 0;
}

void MaterialTextureMatrixLoadProc_Type0(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x4 matrix;
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags_ & 8)
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
    // Build the six nontrivial entries of the matrix by composing rotation,
    // translation and scale in that order (keep only the ones we have data for)
    data_020f1e88[renderData->flags_ & 7](&fifoData.matrix, renderData);
    
    if (renderData->materialxScale_ != 1 << 12)
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[1] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[1]) >> 12;
        fifoData.matrix.entries[12] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[12]) >> 12;
    }

    if (renderData->materialyScale_ != 1 << 12)
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[5] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[5]) >> 12;
        fifoData.matrix.entries[13] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[13]) >> 12;
    }

    SubmitCommandToGeometryFifo(fifoData.commands, (uint32_t*)&fifoData + 1, 18);
}

void BoneMatrixDataSubmissionProc_Type2(BoneMatrixRenderData* renderData)
{
    bool bVar2 = false;
    int flag10orflag8 = renderData->flags_ & 0x18;
    if (!flag10orflag8)
    {
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v2_, 3);
    }
    if (!(renderData->flags_ & 4)) // has translation
    {
        if (flag10orflag8)
            bVar2 = true;
        else
        {
            uint32_t scaledTranslation[3];
            scaledTranslation[0] = ((int64_t)renderData->translate_.x * renderData->scale_v1_.x) >> 12;
            scaledTranslation[1] = ((int64_t)renderData->translate_.y * renderData->scale_v1_.y) >> 12;
            scaledTranslation[2] = ((int64_t)renderData->translate_.z * renderData->scale_v1_.z) >> 12;
            SubmitCommandToGeometryFifo(GXFifoCommand_TranslateMatrix, scaledTranslation, 3);
        }
    }

    if (!(renderData->flags_ & 2)) // has rotation
    {
        if (bVar2) // do rotation then translation
            SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)renderData->rotationMatrix_.entries, 12);
        else
            SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)renderData->rotationMatrix_.entries, 9);
    }
    else if (bVar2)
    {
        SubmitCommandToGeometryFifo(GXFifoCommand_TranslateMatrix, (uint32_t*)&renderData->translate_, 3);
    }

    if (!flag10orflag8)
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v1_, 3);

    if (!(renderData->flags_ & 1)) // has scale
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, (uint32_t*)&renderData->scale_v0_, 3);
}

void BoneMatrixScaleCalculationProc_Type2(BoneMatrixRenderData* renderData, NSBXXBoneMatrix::Scaling* scaling, uint8_t* ip, int boneMatrixFlags)
{
    unsigned int thisBoneIdx = ip[1];
    unsigned int parentBoneIdx = ip[2];
    if (boneMatrixFlags & 4) // no scaling
    {
        renderData->flags_ |= 1; // no scaling
        if (data_0210a274->boneMatrixBitfield_[parentBoneIdx >> 5] & (1 << (parentBoneIdx & 0x1f)))
        {
            data_0210a274->boneMatrixBitfield_[thisBoneIdx >> 5] |= (1 << (thisBoneIdx & 0x1f));
            renderData->flags_ |= 0x18;
        }
        else
        {
            Struct_0210b078* parent = &data_0210b078[parentBoneIdx];
            func_020ca408(parent, &data_0210b078[thisBoneIdx], sizeof(Struct_0210b078));
            // copy to scale2 and scale3
            func_020ca408(parent, &renderData->scale_v1_, 2 * 3 * sizeof(fix32_t));
        }
    }
    else
    {
        renderData->scale_v0_.x = scaling->x;
        renderData->scale_v0_.y = scaling->y;
        renderData->scale_v0_.z = scaling->z;
        if (data_0210a274->boneMatrixBitfield_[parentBoneIdx >> 5] & (1 << (parentBoneIdx & 0x1f)))
        {
            func_020ca408(scaling, &data_0210b078[thisBoneIdx], 2 * 3 * sizeof(fix32_t));
            data_0210a274->boneMatrixBitfield_[thisBoneIdx >> 5] &= ~(1 << (thisBoneIdx & 0x1f));
            renderData->flags_ |= 0x18;
        }
        else
        {
            data_0210a274->boneMatrixBitfield_[thisBoneIdx >> 5] &= ~(1 << (thisBoneIdx & 0x1f));
            data_0210b078[thisBoneIdx].vec0_.x = ((int64_t)scaling->x * (int64_t)data_0210b078[parentBoneIdx].vec0_.x) >> 12;
            data_0210b078[thisBoneIdx].vec0_.y = ((int64_t)scaling->y * (int64_t)data_0210b078[parentBoneIdx].vec0_.y) >> 12;
            data_0210b078[thisBoneIdx].vec0_.z = ((int64_t)scaling->z * (int64_t)data_0210b078[parentBoneIdx].vec0_.z) >> 12;
            data_0210b078[thisBoneIdx].vec1_.x = ((int64_t)scaling->x_v2 * (int64_t)data_0210b078[parentBoneIdx].vec1_.x) >> 12;
            data_0210b078[thisBoneIdx].vec1_.y = ((int64_t)scaling->y_v2 * (int64_t)data_0210b078[parentBoneIdx].vec1_.y) >> 12;
            data_0210b078[thisBoneIdx].vec1_.z = ((int64_t)scaling->z_v2 * (int64_t)data_0210b078[parentBoneIdx].vec1_.z) >> 12;
            func_020ca408(&data_0210b078[parentBoneIdx], &renderData->scale_v1_, 2 * 3 * sizeof(fix32_t));
        }
    }
}

void MaterialTextureMatrixLoadProc_Type1(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x3 matrix; // local_38
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags_ & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x3, GXFifoCommand_SetMatrixMode);
    fifoData.initialMatrixMode = 3; // switch to texture matrix mode
    fifoData.finalMatrixMode = 2; // back to position+vector

    fifoData.matrix.entries[1] = fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[5]
        = fifoData.matrix.entries[6] = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[11] = 0;
    
    // This version only does translation and scaling, translation is negated and
    // applies before scaling.
    if (renderData->flags_ & 4) // no translation data
    {
        fifoData.matrix.entries[9] = 0;
        fifoData.matrix.entries[10] = 0;
        if (renderData->flags_ & 1) // no scaling data
        {
            fifoData.matrix.entries[0] = 1 << 12;
            fifoData.matrix.entries[4] = 1 << 12;
        }
        else
        {
            fifoData.matrix.entries[0] = renderData->extensionScaleX_;
            fifoData.matrix.entries[4] = renderData->extensionScaleY_;
        }
    }
    else // has translation data
    {
        if (renderData->flags_ & 1) // no scaling data
        {
            fix32_t tx = -(renderData->translateX_ * 16);
            fifoData.matrix.entries[9] = tx * renderData->materialWidth_;
            fix32_t ty = -(renderData->translateY_ * 16);
            fifoData.matrix.entries[10] = ty * renderData->materialHeight_;
            fifoData.matrix.entries[0] = 1 << 12;
            fifoData.matrix.entries[4] = 1 << 12;
        }
        else
        {
            fifoData.matrix.entries[9] = renderData->materialWidth_ * -(fix32_t)(((int64_t)renderData->extensionScaleX_ * renderData->translateX_) >> 8);
            fifoData.matrix.entries[10] = renderData->materialHeight_ * -(fix32_t)(((int64_t)renderData->extensionScaleY_ * renderData->translateY_) >> 8);
            fifoData.matrix.entries[0] = renderData->extensionScaleX_;
            fifoData.matrix.entries[4] = renderData->extensionScaleY_;
        }
    }

    if (renderData->materialxScale_ != (1 << 12))
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[9] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[9]) >> 12;
    }

    if (renderData->materialyScale_ != (1 << 12))
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[10] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[10]) >> 12;
    }

    SubmitCommandToGeometryFifo(fifoData.commands, (uint32_t*)&fifoData + 1, 14);
}

extern "C" void CreateTextureMatrix_v2_TranslateRotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationCosine_) >> 12;
    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationSine_) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationCosine_) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationSine_) >> 12;
    
    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    int shiftedTx = -renderData->materialWidth_ * (1 << 11);
    int shiftedTy = -renderData->materialHeight_ * (1 << 11) + renderData->translateY_ * renderData->materialHeight_;
    shiftedTx -= renderData->translateX_ * renderData->materialWidth_;

    matrix->entries[12] = (fix32_t)((((int64_t)scaleXcos * shiftedTx) 
        - ((int64_t)scaleXsin * shiftedTy)) >> 8)
        + renderData->materialWidth_ * (8 << 12);

    matrix->entries[13] = (fix32_t)((((int64_t)scaleYsin * shiftedTx)
        + ((int64_t)scaleYcos * shiftedTy)) >> 8)
        + renderData->materialHeight_ * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-scaleXsin * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v2_TranslateRotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine_;
    matrix->entries[5] = renderData->rotationCosine_;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine_ * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    // these hold (-w/2 - w * t_x) and (-h/2 + h * t_y) respectively
    fix32_t shiftedTx = -renderData->materialWidth_ * (1 << 11);
    fix32_t shiftedTy = -renderData->materialHeight_ * (1 << 11) + renderData->translateY_ * renderData->materialHeight_;
    shiftedTx -= renderData->translateX_ * renderData->materialWidth_;


    matrix->entries[12] = (fix32_t)((((int64_t)renderData->rotationCosine_ * shiftedTx) 
        - ((int64_t)renderData->rotationSine_ * shiftedTy)) >> 8)
        + renderData->materialWidth_ * (8 << 12);
    matrix->entries[13] = (fix32_t)((((int64_t)renderData->rotationSine_ * shiftedTx) 
        + ((int64_t)renderData->rotationCosine_ * shiftedTy)) >> 8)
        + renderData->materialHeight_ * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine_ * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v2_TranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX_;
    matrix->entries[5] = renderData->extensionScaleY_;

    matrix->entries[1] = 0;

    int tx = (-renderData->materialWidth_ * 0x800 - renderData->translateX_ * renderData->materialWidth_);
    int ty = (-renderData->materialHeight_ * 0x800 + renderData->translateY_ * renderData->materialHeight_);

    matrix->entries[12] = (fix32_t)(((int64_t)renderData->extensionScaleX_ * tx) >> 8) + renderData->materialWidth_ * (8 << 12);
    matrix->entries[13] = (fix32_t)(((int64_t)renderData->extensionScaleY_ * ty) >> 8) + renderData->materialHeight_ * (8 << 12);

    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v2_Translate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[5] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[12] = -renderData->translateX_ * renderData->materialWidth_ * 16;
    matrix->entries[13] = renderData->translateY_ * renderData->materialHeight_ * 16;
    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v2_RotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    
    fix32_t scaleXcos = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationCosine_) >> 12;
    fix32_t scaleXsin = ((int64_t)renderData->extensionScaleX_ * (int64_t)renderData->rotationSine_) >> 12;
    fix32_t scaleYcos = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationCosine_) >> 12;
    fix32_t scaleYsin = ((int64_t)renderData->extensionScaleY_ * (int64_t)renderData->rotationSine_) >> 12;
    
    matrix->entries[0] = scaleXcos;
    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    int shiftedTx = renderData->materialWidth_;
    int shiftedTy = renderData->materialHeight_;
    shiftedTy = -shiftedTy * ((1 << 12) >> 1);
    shiftedTx = -shiftedTx * ((1 << 12) >> 1);

    matrix->entries[12] = (fix32_t)((((int64_t)scaleXcos * shiftedTx) 
        - ((int64_t)scaleXsin * shiftedTy)) >> 8)
        + renderData->materialWidth_ * (8 << 12);

    matrix->entries[13] = (fix32_t)((((int64_t)scaleYsin * shiftedTx)
        + ((int64_t)scaleYcos * shiftedTy)) >> 8)
        + renderData->materialHeight_ * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-scaleXsin * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v2_Rotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine_;
    matrix->entries[5] = renderData->rotationCosine_;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine_ * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);

    fix32_t shiftedTx = renderData->materialWidth_;
    fix32_t shiftedTy = renderData->materialHeight_;
    shiftedTy = -shiftedTy * ((1 << 12) >> 1); // holds -height/2 as fix32
    shiftedTx = -shiftedTx * ((1 << 12) >> 1); // holds -width/2 as fix32

    matrix->entries[12] = (fix32_t)((((int64_t)renderData->rotationCosine_ * shiftedTx) 
        - ((int64_t)renderData->rotationSine_ * shiftedTy)) >> 8)
        + renderData->materialWidth_ * (8 << 12);
    matrix->entries[13] = (fix32_t)((((int64_t)renderData->rotationSine_ * shiftedTx) 
        + ((int64_t)renderData->rotationCosine_ * shiftedTy)) >> 8)
        + renderData->materialHeight_ * (8 << 12);

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine_ * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v2_Scale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX_;
    matrix->entries[5] = renderData->extensionScaleY_;

    matrix->entries[1] = 0;

    matrix->entries[12] = ((1 << 12) - renderData->extensionScaleX_) * renderData->materialWidth_ * 8;
    matrix->entries[13] = ((1 << 12) - renderData->extensionScaleY_) * renderData->materialHeight_ * 8;

    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v2_NoExtensions(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[4] = 0;
    matrix->entries[5] = 1 << 12;
    matrix->entries[12] = 0;
    matrix->entries[13] = 0;
}

void MaterialTextureMatrixLoadProc_Type2(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x4 matrix;
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags_ & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x4, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x4, GXFifoCommand_SetMatrixMode);

    fifoData.initialMatrixMode = 3; // load/adjust texture matrix
    fifoData.finalMatrixMode = 2; // return to position+vector mode after

    fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[6]
        = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[9]
        = fifoData.matrix.entries[10] = fifoData.matrix.entries[11] = fifoData.matrix.entries[14] = 0;
    fifoData.matrix.entries[15] = 1 << 12;

    // apply whichever of translation, rotation, scale are available.
    // Note that this time, translation happens first. Also rotation is stupid,
    // the center of rotation is not (8w, 8h) like in v0.
    data_020f1ea8[renderData->flags_ & 7](&fifoData.matrix, renderData);

    if (renderData->materialxScale_ != (1 << 12))
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[1] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[1]) >> 12;
        fifoData.matrix.entries[12] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[12]) >> 12;
    }

    if (renderData->materialyScale_ != (1 << 12))
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[5] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[5]) >> 12;
        fifoData.matrix.entries[13] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[13]) >> 12;
    }

    SubmitCommandToGeometryFifo(fifoData.commands, (uint32_t*)&fifoData + 1, 18);
}

extern "C" void CreateTextureMatrix_v3_ScaleTranslateRotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    fix32_t cosine = renderData->rotationCosine_;
    fix32_t sine = renderData->rotationSine_;
    fix32_t scaleX = renderData->extensionScaleX_;
    fix32_t scaleY = renderData->extensionScaleY_; 

    fix32_t scaleXcos = ((int64_t)scaleX * cosine) >> 12;
    
    matrix->entries[0] = scaleXcos;

    fix32_t scaleXsin = ((int64_t)scaleX * sine) >> 12;
    fix32_t scaleYcos = ((int64_t)scaleY * cosine) >> 12;
    fix32_t scaleYsin = ((int64_t)scaleY * sine) >> 12;    

    matrix->entries[5] = scaleYcos;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (scaleYsin * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    
    int64_t txsin = (int64_t)renderData->translateX_ * renderData->rotationSine_;
    int64_t tysin = (int64_t)renderData->translateY_ * renderData->rotationSine_;
    int64_t txcos = (int64_t)renderData->translateX_ * renderData->rotationCosine_;
    int64_t tycos = (int64_t)renderData->translateY_ * renderData->rotationCosine_;
    
    int64_t rotatedty = (txsin - tycos) >> 12;
    fix32_t scaledrotatedty = (fix32_t)((rotatedty * renderData->extensionScaleY_) >> 12);
    fix32_t m42prescaled = scaleYcos + scaledrotatedty - 0x1000;
    
    int64_t rotatedtx = (txcos + tysin) >> 12;

    fix32_t scaledrotatedtx = (rotatedtx * renderData->extensionScaleX_) >> 12;
    fix32_t m41prescaled = scaleXsin - scaledrotatedtx;
    
    matrix->entries[12] = renderData->materialWidth_ * m41prescaled * 16;
    matrix->entries[13] = -renderData->materialHeight_ * m42prescaled * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-scaleXsin * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v3_TranslateRotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);
    matrix->entries[0] = renderData->rotationCosine_;
    matrix->entries[5] = renderData->rotationCosine_;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine_ * aspectRatio) >> 12;
    fix32_QueueComputeQuotient(matWidth, matHeight);

    int64_t sine = renderData->rotationSine_;
    int64_t cosine = renderData->rotationCosine_;
    int64_t basetx = renderData->translateX_;
    int64_t basety = renderData->translateY_;
    
    int64_t xcos = basetx * cosine;
    int64_t ysin = basety * sine;    

    int32_t tx = (xcos + ysin) >> 12;

    int64_t ycos = basety * cosine;
    int64_t xsin = basetx * sine;
    
    int32_t ty = (xsin - ycos) >> 12;
    
    matrix->entries[12] = renderData->materialWidth_ * (renderData->rotationSine_ - tx) * 16;
    matrix->entries[13] = -renderData->materialHeight_ * (renderData->rotationCosine_ + ty - 0x1000) * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine_ * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v3_TranslateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX_;
    matrix->entries[5] = renderData->extensionScaleY_;
    matrix->entries[1] = 0;

    
    fix32_t scaledTx = ((int64_t)renderData->translateX_ * renderData->extensionScaleX_) >> 12;
    fix32_t scaledTy = ((int64_t)-renderData->translateY_ * renderData->extensionScaleY_) >> 12;

    matrix->entries[12] = renderData->materialWidth_ * -scaledTx * 16;
    matrix->entries[13] = -renderData->materialHeight_ * (renderData->extensionScaleY_ + scaledTy - 0x1000) * 16;

    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v3_Translate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[5] = 1 << 12;
    matrix->entries[1] = 0;

    fix32_t tx = -renderData->translateX_;
    fix32_t ty = -renderData->translateY_;

    matrix->entries[12] = renderData->materialWidth_ * tx * 16;
    matrix->entries[13] = -renderData->materialHeight_ * ty * 16;
    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v3_RotateScale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    fix32_t sine = renderData->rotationSine_;
    fix32_t cosine = renderData->rotationCosine_;

    fix32_t xsin = ((int64_t)renderData->extensionScaleX_ * sine) >> 12;    
    fix32_t xcos = ((int64_t)renderData->extensionScaleX_ * cosine) >> 12;
    
    fix32_t scaleY = renderData->extensionScaleY_;

    fix32_t ycos = ((int64_t)scaleY * cosine) >> 12;

    matrix->entries[0] = xcos;
    matrix->entries[5] = ycos;

    fix32_t aspectRatio = fix32_GetDivisionResult();

    fix32_t ysine = ((int64_t)scaleY * sine) >> 12;
    matrix->entries[1] = (ysine * aspectRatio) >> 12;

    fix32_QueueComputeQuotient(matWidth, matHeight);
    matrix->entries[12] = renderData->materialWidth_ * xsin * 16;
    matrix->entries[13] = -renderData->materialHeight_ * (ycos - 0x1000) * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-xsin * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v3_Rotate(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    fix32_t matWidth = renderData->materialWidth_ << 12;
    fix32_t matHeight = renderData->materialHeight_ << 12;

    fix32_QueueComputeQuotient(matHeight, matWidth);

    matrix->entries[0] = renderData->rotationCosine_;
    matrix->entries[5] = renderData->rotationCosine_;

    fix32_t aspectRatio = fix32_GetDivisionResult();
    matrix->entries[1] = (renderData->rotationSine_ * aspectRatio) >> 12;
    
    fix32_QueueComputeQuotient(matWidth, matHeight);

    matrix->entries[12] = renderData->materialWidth_ * renderData->rotationSine_ * 16;
    matrix->entries[13] = -renderData->materialHeight_ * (renderData->rotationCosine_ - (1 << 12)) * 16;

    fix32_t invAspectRatio = fix32_GetDivisionResult();
    matrix->entries[4] = (-renderData->rotationSine_ * invAspectRatio) >> 12;
}

extern "C" void CreateTextureMatrix_v3_Scale(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = renderData->extensionScaleX_;
    matrix->entries[5] = renderData->extensionScaleY_;
    matrix->entries[1] = 0;
    matrix->entries[12] = 0;
    matrix->entries[13] = -renderData->materialHeight_ * (renderData->extensionScaleY_ - (1 << 12)) * 16;
    matrix->entries[4] = 0;
}

extern "C" void CreateTextureMatrix_v3_NoExtensions(Matrix4x4* matrix, MaterialRenderData* renderData)
{
    matrix->entries[0] = 1 << 12;
    matrix->entries[1] = 0;
    matrix->entries[4] = 0;
    matrix->entries[5] = 1 << 12;
    matrix->entries[12] = 0;
    matrix->entries[13] = 0;
}

void MaterialTextureMatrixLoadProc_Type3(MaterialRenderData* renderData)
{
    struct {
        uint32_t commands;
        uint32_t initialMatrixMode;
        Matrix4x4 matrix;
        uint32_t finalMatrixMode;
    } fifoData;

    if (renderData->flags_ & 8)
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x4, GXFifoCommand_SetMatrixMode);
    else
        fifoData.commands = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_MultiplyMat4x4, GXFifoCommand_SetMatrixMode);

    fifoData.initialMatrixMode = 3; // load/adjust texture matrix
    fifoData.finalMatrixMode = 2; // return to position+vector mode after

    fifoData.matrix.entries[2] = fifoData.matrix.entries[3] = fifoData.matrix.entries[6]
        = fifoData.matrix.entries[7] = fifoData.matrix.entries[8] = fifoData.matrix.entries[9]
        = fifoData.matrix.entries[10] = fifoData.matrix.entries[11] = fifoData.matrix.entries[14] = 0;
    fifoData.matrix.entries[15] = 1 << 12;

    if (renderData->flags_ & 1)
    {
        renderData->extensionScaleX_ = renderData->extensionScaleY_ = 1 << 12;
    }

    if (renderData->flags_ & 2)
    {
        renderData->rotationCosine_ = 1 << 12;
        renderData->rotationSine_ = 0;
    }

    if (renderData->flags_ & 4)
    {
        renderData->translateX_ = renderData->translateY_ = 0;
    }

    data_020f1ec8[renderData->flags_ & 7](&fifoData.matrix, renderData);

    if (renderData->materialxScale_ != (1 << 12))
    {
        fifoData.matrix.entries[0] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[0]) >> 12;
        fifoData.matrix.entries[1] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[1]) >> 12;
        fifoData.matrix.entries[12] = ((int64_t)renderData->materialxScale_ * fifoData.matrix.entries[12]) >> 12;
    }

    if (renderData->materialyScale_ != (1 << 12))
    {
        fifoData.matrix.entries[4] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[4]) >> 12;
        fifoData.matrix.entries[5] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[5]) >> 12;
        fifoData.matrix.entries[13] = ((int64_t)renderData->materialyScale_ * fifoData.matrix.entries[13]) >> 12;
    }

    SubmitCommandToGeometryFifo(fifoData.commands, (uint32_t*)&fifoData + 1, 18);
}