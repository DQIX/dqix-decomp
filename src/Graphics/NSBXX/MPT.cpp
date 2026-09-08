#include "Graphics/NSBXX/NSBXX.h"
#include "Graphics/NSBXX/RenderCommands_Common.h"
#include "Graphics/NSBXX/Animation.h"

#pragma optimize_for_size off

#if defined(jpn)
#define func_020ca390 func_020cbe5c
#define data_020f1c7c data_020f1de8
#endif

extern "C"
{
    // memset
    void func_020ca390(int, void*, unsigned);
}

// processing callback for M.PT animations
extern void (*data_020f1c7c)(void*, AnimationData*, int);

void InitializeModelAnimationFromMPT(AnimationData* anim, void* pvMPT, NSBXXInternalModel* model)
{
    unsigned int track;
    NSBXXModelMaterialData* materials;
    if (model != NULL && model->materialsOffset_ != 0)
        materials = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materials = NULL;
    
    anim->callback_ = data_020f1c7c;

    anim->numEntries_ = model->numMaterials_;
    NSBXXAnimationMPT* mpt = (NSBXXAnimationMPT*)pvMPT;
    anim->pRawData_ = mpt;
    func_020ca390(0, &anim->entries_[0], anim->numEntries_ * 2);
    
    track = 0;
    if (track < mpt->tracks_.numEntries_)
    {
        unsigned int bufferOffset = 0;
        do
        {
            const char* trackName = mpt->tracks_.GetNameByIndexAndOffset(track, bufferOffset);
            int idx;

            if (materials == NULL)
                idx = -1;
            else
                idx = NSBXXNameList_SearchIndex(&materials->materialOffsetList_, trackName);

            if (idx >= 0)
            {
                anim->entries_[idx] = track | 0x100;
            }
            track++;
            bufferOffset += 16;
        } while (track < mpt->tracks_.numEntries_);
    }
}

void SetMaterialTextureForRender(NSBXXTex* tex, const char* textureName, MaterialRenderData* renderData)
{
    NSBXXTexTexture* innerTexture = tex ? (NSBXXTexTexture*)NSBXXNameList_Search(&tex->textureList_, textureName) : NULL;
    unsigned int vramOffset;
    if ((innerTexture->paramTEXIMAGE_PARAMS_ & 0x1c000000) != 0x14000000)
        vramOffset = (unsigned short)tex->block1VRAMLoadOffset_ & ~0xe0000000;
    else
        vramOffset = (unsigned short)tex->block2Or3VRAMLoadOffset_ & ~0xe0000000;

    // extract the transform mode (top 2 bits) and flip/reflect settings (bits 16-19)
    renderData->paramTEXIMAGE_PARAMS_ &= 0xc00f0000;
    renderData->paramTEXIMAGE_PARAMS_ |= innerTexture->paramTEXIMAGE_PARAMS_ + vramOffset;
    
    renderData->materialWidth_ = innerTexture->unk_4 & 0x7ff;
    renderData->materialHeight_ = (innerTexture->unk_4 & 0x3ff800) >> 11;


    int newWidth = innerTexture->unk_4 & 0x7ff;
    int newHeight = (innerTexture->unk_4 & 0x3ff800) >> 11;

    // wtf is the point of this? you just set it to these values...
    fix32_t rescaleX = (newWidth == renderData->materialWidth_) ? 0x1000 : fix32_Divide(newWidth << 12, renderData->materialWidth_ << 12);
    renderData->materialxScale_ = rescaleX;

    fix32_t rescaleY = (newHeight == renderData->materialHeight_) ? 0x1000 : fix32_Divide(newHeight << 12, renderData->materialHeight_ << 12);
    renderData->materialyScale_ = rescaleY;
}

void SetMaterialPaletteForRender(NSBXXTex* tex, const char* paletteName, MaterialRenderData* renderData)
{
    NSBXXTexPalette* palette;
    if (tex != NULL && tex->paletteListOffset_ != 0)
    {
        NSBXXNameList* paletteList = (NSBXXNameList*)((intptr_t)tex + tex->paletteListOffset_);
        palette = (NSBXXTexPalette*)NSBXXNameList_Search(paletteList, paletteName);
    }
    else
    {
        palette = NULL;
    }

    
    unsigned short blockStartPos = (unsigned short)tex->block4VRAMLoadOffset_ & ~0xe0000000;
    unsigned short offsetInBlock = palette->offsetWithinBlock4_;
    if (!(palette->unk_2 & 1))
    {
        offsetInBlock >>= 1; 
        blockStartPos >>= 1;
    }

    renderData->texturePaletteBase_ = offsetInBlock + blockStartPos;
}

void MPTAnimationProcessingCallback(void* data, AnimationData* anim, int arg)
{
    MaterialRenderData* renderData = (MaterialRenderData*)data;

    NSBXXAnimationMPT* mpt = (NSBXXAnimationMPT*)anim->pRawData_;

    NSBXXAnimationMPT::Track::Keyframe* keyframe = NSBXX_PatternAnimation_GetKeyframe(mpt, arg, anim->time_ >> 12);
    const char* textureName = NSBXX_PatternAnimation_GetTextureName(mpt, keyframe->textureIdx_);
    SetMaterialTextureForRender(anim->pTex0_, textureName, renderData);

    if (keyframe->paletteIdx_ != 0xff)
    {
        const char* paletteName = NSBXX_PatternAnimation_GetPaletteName(mpt, keyframe->paletteIdx_);
        SetMaterialPaletteForRender(anim->pTex0_, paletteName, renderData);
    }
}