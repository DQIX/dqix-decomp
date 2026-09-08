#include "Graphics/Model3D.h"
#include "Graphics/NSBXX/NSBXX.h"
#include "Graphics/NSBXX/RenderCommands.h"
#include "Graphics/Vector.h"
#include "Graphics/VRAMStaging.h"
#include "System/Cache.h"
#include "System/Graphics.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_020f1ee8 data_020f2054
#define data_020f1eec data_020f2058
#define data_020f1ef0 data_020f205c
#define data_020f1ef4 data_020f2060

#define func_020c56b0 func_020c717c
#endif

extern "C"
{
    int func_020c56b0(int*);
}

extern unsigned int (*data_020f1ee8)(unsigned int, int, int);
extern unsigned int (*data_020f1eec)(unsigned int);
extern unsigned int (*data_020f1ef0)(unsigned int, int, int);
extern unsigned int (*data_020f1ef4)(unsigned int);

void Model3D::Clear()
{
    unknown_flags_a8_0_ = false;
    rawInternalModel_ = NULL;
    rawTEX_ = NULL;
    rawBMD_ = NULL;
    unknown_9c_ = 0;
    unknown_a0_ = 0x1000;
    unknown_98_ = 0;
    pBoneMatrixRenderData_ = 0;
    pMaterialRenderData_ = 0;
    alpha_ = 0x1f;
    unknown_flags_a8_1_ = false;
    unknown_flags_a8_2_ = false;
    imageStagingTaskID_ = 0xffff;
    paletteStagingTaskID_ = 0xffff;
    memset(this, 0, 0x54); // substruct
    memset(&xMax_, 0, 6 * sizeof(fix32_t)); 
}

void Model3D::Func0207e2e0()
{
    CancelVRAMStagingOperation(paletteStagingTaskID_);
    CancelVRAMStagingOperation(imageStagingTaskID_);
    this->Clear();
}

void Model3D::LoadFromFile(const char* path, AllocatorUnion* alloc, TextureStagingMode stagingMode)
{
    if (alloc == NULL)
        return;
    BackgroundLoader::AddLockGlobal();
    unsigned int length;
    void* scratchData = LoadFileIntoMemory(path, &data_0211e33c, &length);
    if (scratchData == NULL)
    {
        unknown_flags_a8_0_ = false;
    }
    else
    {
        CleanInvalidateCacheRange(scratchData, length);
        CopyAndProcessRawFile(alloc, scratchData, length, stagingMode);
    }
    BackgroundLoader::RemoveLockGlobal();
}

void Model3D::CopyAndProcessRawFile(AllocatorUnion* alloc, const void* data,
    unsigned int length, TextureStagingMode stagingMode)
{
    if (alloc == NULL || data == NULL || length == 0)
        return;

    CopyRawFile(alloc, data, length);
    CleanInvalidateCacheRange(rawBMD_, rawBMDFileSize_);
    ProcessRawFile(stagingMode);
}

void Model3D::SetAndProcessRawFile(void* data, unsigned int length, TextureStagingMode stagingMode)
{
    rawBMD_ = data;
    rawBMDFileSize_ = length;
    CleanInvalidateCacheRange(rawBMD_, rawBMDFileSize_);
    ProcessRawFile(stagingMode);
}

void Model3D::CopyRawFile(AllocatorUnion* alloc, const void* data, unsigned int length)
{
    rawBMD_ = alloc->Allocate(length);
    if (rawBMD_ == NULL)
    {
        this->Clear();
    }
    else
    {
        memcpy(rawBMD_, data, length);
        rawBMDFileSize_ = length;
    }
}

void Model3D::SetRawFile(void* data, unsigned int length)
{
    rawBMD_ = data;
    rawBMDFileSize_ = length;
}

void Model3D::ClearRawFileCache()
{
    CleanInvalidateCacheRange(rawBMD_, rawBMDFileSize_);
}

void Model3D::ProcessRawFile(TextureStagingMode stagingMode)
{
    bool block4Succeeded, block23Succeeded, block1Succeeded;
    NSBXXTex* tex0;
    NSBXXContainer* data = (NSBXXContainer*)rawBMD_;
    if (data == NULL)
        return;
    switch (data->signature_)
    {
    case SIGNATURE_NSBVA:
    case SIGNATURE_NSBTA:
    case SIGNATURE_NSBMA:
    case SIGNATURE_NSBCA:
    case SIGNATURE_NSBTP:
        break;
    case SIGNATURE_NSBMD:
    case SIGNATURE_NSBTX:
    {
        block1Succeeded = true;
        block23Succeeded = true;
        block4Succeeded = true;
        
        tex0 = (NSBXXTex*)NSBXX_GetTEXFile(data);
        if (tex0 != NULL)
        {
            unsigned int block4Alloc, block2Alloc, block1Alloc;

            unsigned int block1Len = NSBXX_Tex_GetBlock1Length(tex0);
            unsigned int block2Len = NSBXX_Tex_GetBlock2Length(tex0);
            unsigned int block4Len = NSBXX_Tex_GetBlock4Length(tex0);            

            if (block1Len != 0)
            {
                block1Alloc = data_020f1ee8(block1Len, 0, 0);
                if (block1Alloc == 0)
                    block1Succeeded = false;
            }
            else
                block1Alloc = 0;

            if (block2Len != 0)
            {
                block2Alloc = data_020f1ee8(block2Len, 1, 0);
                if (block2Alloc == 0)
                    block23Succeeded = false;
            }
            else
                block2Alloc = 0;
            
            if (block4Len != 0)
            {
                block4Alloc = data_020f1ef0(block4Len, tex0->maybeBlock23Flags_20_ & 0x8000, 0);
                if (block4Alloc == 0)
                    block4Succeeded = false;
            }
            else
                block4Alloc = 0;

            if (!block1Succeeded || !block23Succeeded || !block4Succeeded)
            {
                if (block4Succeeded)
                    data_020f1ef4(block4Alloc);
                if (block23Succeeded)
                    data_020f1eec(block2Alloc);
                if (block1Succeeded)
                    data_020f1eec(block1Alloc);
                break; // from switch statement
            }
            else
            {
                NSBXX_Tex_WriteImageVRAMOffsets(tex0, block1Alloc, block2Alloc);
                NSBXX_Tex_WritePaletteVRAMOffset(tex0, block4Alloc);
            }
        }
        if (data->signature_ == SIGNATURE_NSBMD)
        {
            NSBXXMdl* mdl0 = (NSBXXMdl*)NSBXX_GetFirstSubfile(data);
            if (tex0 != NULL)
                NSBXX_LinkTEX0ToMDL0(mdl0, tex0);
        }
        break;
    }
    }
    NSBXXTex* tex = (NSBXXTex*)NSBXX_GetTEXFile((NSBXXContainer*)rawBMD_);
    if (tex != NULL)
    {
        if (stagingMode == TextureStagingMode_Immediate)
        {
            LockStagedTextureVRAMCopying();
            NSBXX_Tex_LoadPaletteToVRAM(tex, true);
            NSBXX_Tex_LoadImageToVRAM(tex, true);
            UnlockStagedTextureVRAMCopying();
        }
        else
        {
            paletteStagingTaskID_ = StageTexFilePaletteData(tex, stagingMode == TextureStagingMode_HighPriority);
            imageStagingTaskID_ = StageTexFileImageData(tex, stagingMode == TextureStagingMode_HighPriority);
        }
    }

    NSBXXMdl* mdl0 = (NSBXXMdl*)NSBXX_GetFirstSubfile((NSBXXContainer*)rawBMD_);
    NSBXXInternalModel* model;
    if (mdl0 != NULL)
    {
        NSBXXNameList* nameList = &mdl0->nameList_;
        intptr_t pModelOffset;
        unsigned int zero = 0; // prevents compiler from optimising > 0 to != 0
        if (nameList != NULL && mdl0->nameList_.numEntries_ > zero)
        {
            pModelOffset = (intptr_t)nameList + mdl0->nameList_.offsetToDataStart_ + 4;
        }
        else
            pModelOffset = 0;
        
        if (pModelOffset != NULL)
        {
            model = (NSBXXInternalModel*)((intptr_t)mdl0 + *(int*)pModelOffset);
            goto found_a_model;
        }
    }

    model = NULL;
found_a_model:
    rawInternalModel_ = model;
    
    PopulateModelRenderContext(&renderContext_, model);
    rawTEX_ = (NSBXXTex*)NSBXX_GetTEXFile((NSBXXContainer*)rawBMD_);
    if (((NSBXXContainer*)rawBMD_)->signature_ == SIGNATURE_NSBMD)
    {
        NSBXXInternalModel* model = rawInternalModel_;
        int32_t scale = model->maybeScale_;

        int32_t scaledXMin = FIX32_MULTIPLY(model->bounds_.xMin_, scale);
        int32_t scaledYMin = FIX32_MULTIPLY(model->bounds_.yMin_, scale);
        int32_t scaledZMin = FIX32_MULTIPLY(model->bounds_.zMin_, scale);

        int32_t scaledXSize = FIX32_MULTIPLY(model->bounds_.xSize_, scale);
        int32_t scaledYSize = FIX32_MULTIPLY(model->bounds_.ySize_, scale);
        int32_t scaledZSize = FIX32_MULTIPLY(model->bounds_.zSize_, scale);

        xMax_ = scaledXMin + scaledXSize;
        yMax_ = scaledYMin + scaledYSize;
        zMax_ = scaledZMin + scaledZSize;

        xMin_ = scaledXMin;
        yMin_ = scaledYMin;
        zMin_ = scaledZMin;

        xMiddle_ = scaledXMin + (scaledXSize / 2);
        maybeYBase_ = scaledYMin;
        zMiddle_ = scaledZMin + (scaledZSize / 2);
        copyOfHeight_ = scaledYSize;

        Vector3fix span = { 0, 0, 0 };
        span.x = scaledXSize;
        span.z = scaledZSize;
        maybeApproxRadius_ = Vector3fix_Length(&span) / 2;
    }
    unknown_flags_a8_0_ = true;
}

bool Model3D::Draw(bool applyClipping)
{
    if (!unknown_flags_a8_0_)
        return false;

    if (applyClipping && !TestVisible())
        return false;

    if (unknown_flags_a8_2_)
    {
        renderContext_.flags_ |= 1;
        RenderModelFromRenderData(&renderContext_);
        renderContext_.flags_ &= ~1;
        unknown_flags_a8_2_ = false;
    }
    else if (unknown_flags_a8_1_)
    {
        renderContext_.flags_ |= 1;
        RenderModelFromRenderData(&renderContext_);
    }
    else
        RenderModelFromRenderData(&renderContext_);
    return true;
}

bool Model3D::DrawMeshWithMaterial(bool applyClipping, unsigned int material, unsigned int mesh, int bind)
{
    if (!unknown_flags_a8_0_)
        return false;

    if (applyClipping && !TestVisible())
        return false;

    RenderMeshWithMaterial(rawInternalModel_, material, mesh, bind);
    return true;
}

int Model3D::TestVisible()
{
    GXFIFO_MATRIX_PUSH = 0;
    int scale = rawInternalModel_->maybeScale_;
    // presumably 3 times for each of the components
    GXFIFO_MATRIX_SCALE = scale;
    GXFIFO_MATRIX_SCALE = scale;
    GXFIFO_MATRIX_SCALE = scale;
    // respectively, these mean
    // render 1-dot polygons behind DISP_1DOT_DEPTH (?)
    // render polygons intersecting the far-plane
    // render front surface
    // render back surface
    // enable light 0 (but not 1, 2 or 3)
    GXFIFO_POLYGON_ATTRIBUTES = (1 << 13) | (1 << 12) | (1 << 7) | (1 << 6) | (1 << 0);
    GXFIFO_POLYGON_BEGIN = 0;
    GXFIFO_POLYGON_END = 0;
    NSBXXInternalModel* model = rawInternalModel_;

    union {
        ModelBoundingBox box;
        struct {
            uint32_t a;
            uint32_t b;
            uint32_t c;
        } u32s;
    } boxparams;

    boxparams.box.xMin_ = model->bounds_.xMin_;
    boxparams.box.yMin_ = model->bounds_.yMin_;
    boxparams.box.zMin_ = model->bounds_.zMin_;
    boxparams.box.xSize_ = model->bounds_.xSize_;
    boxparams.box.ySize_ = model->bounds_.ySize_;
    boxparams.box.zSize_ = model->bounds_.zSize_;

    // we pass two parameters at a time here
    // (xmin, ymin), (zmin, xsize), (ysize, zsize)
    GXFIFO_TEST_BOX = boxparams.u32s.a;
    GXFIFO_TEST_BOX = boxparams.u32s.b;
    GXFIFO_TEST_BOX = boxparams.u32s.c;

    int result;
    while (func_020c56b0(&result) != 0) {}
    GXFIFO_MATRIX_POP = 1;
    return result;
}

int Model3D::GetBoneIndex(const char *boneName)
{
    if (rawInternalModel_ == NULL)
        return -1;

    char zeroPaddedName[16];
    memset(zeroPaddedName, 0, sizeof(zeroPaddedName));
    strcpy(zeroPaddedName, boneName);
    return NSBXXNameList_SearchIndex(&rawInternalModel_->boneList_, zeroPaddedName);
}

void Model3D::ApplyTexturesFromModel(Model3D* otherModel)
{
    if (!unknown_flags_a8_0_ || otherModel == NULL)
        return;
    NSBXXMdl* mdl0 = GetMDL0();
    NSBXXTex* tex0 = (NSBXXTex*)otherModel->rawTEX_;
    if (mdl0 == NULL || tex0 == NULL)
        return;
    NSBXX_LinkTEX0ToMDL0(mdl0, tex0);
}

NSBXXMdl* Model3D::GetMDL0()
{
    if (rawBMD_ == NULL)
        return NULL;
    return (NSBXXMdl*)NSBXX_GetFirstSubfile((NSBXXContainer*)rawBMD_);
}

void Model3D::RemoveTextures()
{
    NSBXXMdl* mdl0 = GetMDL0();
    if (mdl0 == NULL)
        return;
    NSBXX_UnlinkTEX0FromMDL0(mdl0);
}

void Model3D::CreateBoneMatrixAndMaterialArrays(SafeAllocator *alloc, int args)
{
    if (alloc != NULL)
    {
        if (pBoneMatrixRenderData_ == NULL && (args & 1))
            pBoneMatrixRenderData_ = (BoneMatrixRenderData*)alloc->Allocate(renderContext_.internalModel_->numBoneMatrices_ * sizeof(BoneMatrixRenderData));

        if (pMaterialRenderData_ == NULL && (args & 2))
            pMaterialRenderData_ = (MaterialRenderData*)alloc->Allocate(renderContext_.internalModel_->numMaterials_ * sizeof(MaterialRenderData));
    }

    if (!(args & 4))
    {
        if (pBoneMatrixRenderData_ != NULL)
            renderContext_.boneMatrixRenderDataArray_ = pBoneMatrixRenderData_;
        if (pMaterialRenderData_ != NULL)
            renderContext_.materialRenderDataArray_ = pMaterialRenderData_;

        unknown_flags_a8_2_ = true;
    }
}

void Model3D::StoreBoneMatrixAndMaterialArrayPointers()
{
    if (pBoneMatrixRenderData_ != NULL)
        renderContext_.boneMatrixRenderDataArray_ = pBoneMatrixRenderData_;
    if (pMaterialRenderData_ != NULL)
        renderContext_.materialRenderDataArray_ = pMaterialRenderData_;
    unknown_flags_a8_1_ = true;
}

void Model3D::SetAlpha(int alpha)
{
    if (rawInternalModel_ != NULL)
    {
        NSBXX_Model_SetAlpha(rawInternalModel_, alpha);
        alpha_ = alpha;
    }
}

int Model3D::GetAlpha() const { return alpha_; }

void Model3D::AddAnimation(Animation3D* anim)
{
    if (anim == NULL || anim->GetBasicAnimationData() == NULL)
        return;

    AddAnimationsToModelRenderContext(&renderContext_, anim->GetBasicAnimationData());
}

void Model3D::RemoveAnimations()
{
    while (renderContext_.materialAnimations_ != NULL)
        RemoveAnimationFromModelRenderContext(&renderContext_, renderContext_.materialAnimations_);

    while (renderContext_.jointAnimations_ != NULL)
        RemoveAnimationFromModelRenderContext(&renderContext_, renderContext_.jointAnimations_);

    while (renderContext_.visibilityAnimations_ != NULL)
        RemoveAnimationFromModelRenderContext(&renderContext_, renderContext_.visibilityAnimations_);
}

void Model3D::SetBoneMatrixRenderData(BoneMatrixRenderData* data)
{
    pBoneMatrixRenderData_ = data;
}

BoneMatrixRenderData* Model3D::GetBoneMatrixRenderData()
{
    return pBoneMatrixRenderData_;
}

void Model3D::ApplyRenderConfigMaterialDiffuseColor()
{
    if (rawInternalModel_ != NULL)
        NSBXX_Model_SetAllMaterialFlags(rawInternalModel_, false, 1 << 6);
}

void Model3D::SetMaterialDiffuseColor(unsigned int rgb)
{
    if (rawInternalModel_ != NULL)
        NSBXX_Model_SetDiffuseReflectionColor(rawInternalModel_, rgb);
}

NSBXXTex* Model3D::GetTEX0()
{
    return rawTEX_;
}

void Model3D::SetTEX0(NSBXXTex* tex)
{
    rawTEX_ = tex;
}

void Model3D::RestageTexturePalette()
{
    CancelVRAMStagingOperation(paletteStagingTaskID_);
    paletteStagingTaskID_ = StageTexFilePaletteData(rawTEX_, false);
}

void Model3D::RestageTextureImage()
{
    CancelVRAMStagingOperation(imageStagingTaskID_);
    imageStagingTaskID_ = StageTexFileImageData(rawTEX_, false);
}