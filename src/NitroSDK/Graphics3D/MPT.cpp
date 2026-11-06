#include "NitroSDK/Graphics3D/Internal.h"

#if defined(jpn)
#define func_020ca390 func_020cbe5c
#endif

extern "C"
{
    // memset
    void func_020ca390(int, void*, unsigned);
}

void Nitro3Di_InitAnimationFromMPT(Nitro3DAnimation* anim, void* pvMPT, Nitro3DModel* model)
{
    Nitro3DModelMaterials* materials = model->GetMaterialData();
    anim->callback = g_pfnNitro3DProcessMPT;
    anim->numEntries = model->numMaterials;
    Nitro3DMPT* mpt = (Nitro3DMPT*)pvMPT;
    anim->pRawData = mpt;
    func_020ca390(0, &anim->entries[0], anim->numEntries * 2);
    
    for (unsigned int track = 0; track < mpt->tracks.numEntries; track++)
    {
        const char* trackName = mpt->tracks.GetNameByIndex(track);
        int idx;

        if (materials == NULL)
            idx = -1;
        else
            idx = Nitro3D_GetNameListIndex(&materials->materialOffsetList, trackName);

        if (idx >= 0)
            anim->entries[idx] = track | 0x100;
    }
}

static void SetMaterialTexture(Nitro3DTextureSet* tex, const char* textureName, MaterialRenderData* renderData)
{
    Nitro3DTextureSet::ImageData* image = tex ? (Nitro3DTextureSet::ImageData*)Nitro3D_GetNameListEntry(&tex->textureList, textureName) : NULL;
    unsigned int vramOffset;
    if ((image->paramTEXIMAGE_PARAMS & 0x1c000000) != 0x14000000)
        vramOffset = (unsigned short)tex->block1VRAMLoadOffset & ~0xe0000000;
    else
        vramOffset = (unsigned short)tex->block2Or3VRAMLoadOffset & ~0xe0000000;

    // extract the transform mode (top 2 bits) and flip/reflect settings (bits 16-19)
    renderData->paramTEXIMAGE_PARAMS &= 0xc00f0000;
    renderData->paramTEXIMAGE_PARAMS |= image->paramTEXIMAGE_PARAMS + vramOffset;
    
    renderData->materialWidth = image->dimensions & 0x7ff;
    renderData->materialHeight = (image->dimensions & 0x3ff800) >> 11;


    int newWidth = image->dimensions & 0x7ff;
    int newHeight = (image->dimensions & 0x3ff800) >> 11;

    // condition always evaluates to true...
    fix32_t rescaleX = (newWidth == renderData->materialWidth) ? 0x1000 : fix32_Divide(newWidth << 12, renderData->materialWidth << 12);
    renderData->materialScaleX = rescaleX;

    fix32_t rescaleY = (newHeight == renderData->materialHeight) ? 0x1000 : fix32_Divide(newHeight << 12, renderData->materialHeight << 12);
    renderData->materialScaleY = rescaleY;
}

static void SetMaterialPalette(Nitro3DTextureSet* tex, const char* paletteName, MaterialRenderData* renderData)
{
    Nitro3DTextureSet::PaletteData* palette;
    if (tex != NULL && tex->paletteListOffset != 0)
    {
        Nitro3DNameList* paletteList = (Nitro3DNameList*)((intptr_t)tex + tex->paletteListOffset);
        palette = (Nitro3DTextureSet::PaletteData*)Nitro3D_GetNameListEntry(paletteList, paletteName);
    }
    else
        palette = NULL;

    unsigned short blockStartPos = (unsigned short)tex->block4VRAMLoadOffset & ~0xe0000000;
    unsigned short offsetInBlock = palette->offsetWithinBlock4;
    if (!(palette->unknown_2 & 1))
    {
        offsetInBlock >>= 1; 
        blockStartPos >>= 1;
    }

    renderData->texturePaletteBase = offsetInBlock + blockStartPos;
}

void Nitro3Di_ProcessAnimationMPT(void* data, Nitro3DAnimation* anim, int arg)
{
    MaterialRenderData* renderData = (MaterialRenderData*)data;

    Nitro3DMPT* mpt = (Nitro3DMPT*)anim->pRawData;

    Nitro3DMPT::Track::Keyframe* keyframe = Nitro3Di_GetPatternAnimationKeyframe(mpt, arg, anim->time >> 12);
    const char* textureName = Nitro3Di_GetPatternAnimationTextureName(mpt, keyframe->textureIdx);
    SetMaterialTexture(anim->pTex0, textureName, renderData);

    if (keyframe->paletteIdx != 0xff)
    {
        const char* paletteName = Nitro3Di_GetPatternAnimationPaletteName(mpt, keyframe->paletteIdx);
        SetMaterialPalette(anim->pTex0, paletteName, renderData);
    }
}