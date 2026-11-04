#include "NitroSDK/Graphics3D/Graphics3D.h"
#include "NitroSDK/LoadToVRAM.h"
#include <globaldefs.h>
#include <asmhacks.h>

int NSBXX_Tex_GetBlock1Length(Nitro3DTextureSet* tex)
{
    return (tex != NULL) ? tex->block1NumEightBytes_ << 3 : 0;
}

int NSBXX_Tex_GetBlock2Length(Nitro3DTextureSet* tex)
{
    return (tex != NULL) ? tex->block2NumEightBytes_ << 3 : 0;
}

void NSBXX_Tex_WriteImageVRAMOffsets(Nitro3DTextureSet* tex, int block1, int block2_3)
{
    if (block1 != 0)
        tex->block1VRAMLoadOffset_ = block1;
    if (block2_3 != 0)
        tex->block2Or3VRAMLoadOffset_ = block2_3;
}

void NSBXX_Tex_LoadImageToVRAM(Nitro3DTextureSet* tex, bool needsMapping)
{
    if (needsMapping)
        MemoryMapTextureImage();
    unsigned int block1Length = tex->block1NumEightBytes_ << 3;
    if (block1Length != 0)
    {
        LoadToTextureImage((const void*)((intptr_t)tex + tex->block1Offset_),
            (uint16_t)tex->block1VRAMLoadOffset_ << 3, block1Length);
        tex->maybeBlock1Flags_10_ |= (1 << 0);
    }
    int block3SourceOffset;
    unsigned int block2Length = tex->block2NumEightBytes_ << 3;
    if (block2Length != 0)
    {
        block3SourceOffset = tex->block3Offset_;
        unsigned int block2WriteOffset = (tex->block2Or3VRAMLoadOffset_ << 16) >> 13;
        int block2SourceOffset = tex->block2Offset_;
        
        LoadToTextureImage((const void*)((intptr_t)tex + block2SourceOffset),
            block2WriteOffset, block2Length);
        // I think the idea here is as follows: texture image data consists
        // of some of VRAM units A,B,C,D, which are each 0x20000 bytes.
        // If offset & 0x40000 == 0 then block 2 is put into the first
        // available unit and block 3 in the second, while if offset & 0x40000 != 0
        // then block 2 is put into the third unit and block 3 in the fourth.
        // I'm assuming the 0x20000 bit won't be set, as then things will be weird
        // (though I guess you could have block 2 in second unit and block 3
        // in latter half of the third).
        LoadToTextureImage((const void*)((intptr_t)tex + block3SourceOffset),
            (block2WriteOffset & 0x1ffff) / 2 + 0x20000 + ((block2WriteOffset & 0x40000) >> 2),
            block2Length >> 1);
        tex->maybeBlock23Flags_20_ |= (1 << 0);
    }
    if (needsMapping)
        MemoryUnmapTextureImage();
}

int NSBXX_Tex_GetBlock4Length(Nitro3DTextureSet* tex)
{
    return (tex != NULL) ? tex->block4NumEightBytes_ << 3 : 0;
}

void NSBXX_Tex_WritePaletteVRAMOffset(Nitro3DTextureSet* tex, int offset)
{
    tex->block4VRAMLoadOffset_ = offset;
}

void NSBXX_Tex_LoadPaletteToVRAM(Nitro3DTextureSet* tex, bool needsMapping)
{
    if (needsMapping)
        MemoryMapTexturePalette();

    LoadToTexturePalette((const void*)((intptr_t)tex + tex->block4Offset_),
        (uint16_t)tex->block4VRAMLoadOffset_ << 3,
        tex->block4NumEightBytes_ << 3);
    tex->maybeBlock4Flags_32_ |= (1 << 0);

    if (needsMapping)
        MemoryUnmapTexturePalette();
}

extern "C" void AttachTextureImageToModelMaterials(
    Nitro3DModelMaterials* materialData, Nitro3DModelMaterials::Pair* pairing,
    Nitro3DTextureSet* tex0, Nitro3DTextureSet::ImageData* texture)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray_;
    unsigned int tex0VramLoadOffset;

    if ((texture->paramTEXIMAGE_PARAMS_ & 0x1c000000) != 0x14000000)
        tex0VramLoadOffset = (unsigned short)tex0->block1VRAMLoadOffset_ & ~0xe0000000;
    else
        tex0VramLoadOffset = (unsigned short)tex0->block2Or3VRAMLoadOffset_ & ~0xe0000000;

    
    for (unsigned int i = 0; i < pairing->arraySize_; i++)
    {
        Nitro3DMaterial* material = materialData->GetMaterialByIndex(indexArray[i]);
        material->paramTEXIMAGE_PARAMS_ |= (texture->paramTEXIMAGE_PARAMS_ + tex0VramLoadOffset);
        
        int trueTextureWidth = texture->unk_4 & 0x7ff;
        int trueTextureHeight = (texture->unk_4 & 0x3ff800) >> 11;

        if (trueTextureWidth == material->width_)
            material->xScale_ = 1 << 12;
        else
            material->xScale_ = fix32_Divide(trueTextureWidth << 12, material->width_ << 12);

        if (trueTextureHeight == material->height_)
            material->yScale_ = 1 << 12;
        else
            material->yScale_ = fix32_Divide(trueTextureHeight << 12, material->height_ << 12);
    }
    
    pairing->flags_ |= 1;
}

extern "C" void DetachTextureImageFromModelMaterials(Nitro3DModelMaterials* materialData, Nitro3DModelMaterials::Pair* pairing)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray_;
    for (unsigned int i = 0; i < pairing->arraySize_; i++)
    {
        Nitro3DMaterial* material = materialData->GetMaterialByIndex(indexArray[i]);
        // keep only bits 30-31 and 16,17,18,19
        material->paramTEXIMAGE_PARAMS_ &= 0xc00f0000;
        material->xScale_ = 1 << 12;
        material->yScale_ = 1 << 12;
    }
    pairing->flags_ &= ~1;
}

extern "C" bool NSBXX_AttachTextureImageToModel(Nitro3DModel* model, Nitro3DTextureSet* tex0)
{
    Nitro3DModelMaterials* materialData = model->GetMaterialData();
    Nitro3DNameList* imagePairingsList = (Nitro3DNameList*)((const char*)materialData + materialData->texturePairingsOffset_);
    
    unsigned int i;
    bool success = true;
    for (i = 0; i < imagePairingsList->numEntries_; i++)
    {
        const char* textureName = imagePairingsList->GetNameByIndex(i);
        Nitro3DTextureSet::ImageData* texture = (tex0 != NULL) ?
            (Nitro3DTextureSet::ImageData*)NSBXXNameList_Search(&tex0->textureList_, textureName) : NULL;
        if (texture != NULL)
        {
            Nitro3DModelMaterials::Pair* pairing = imagePairingsList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);

            if (!(pairing->flags_ & 1))
                AttachTextureImageToModelMaterials(materialData, pairing, tex0, texture);
        }
        else
            success = false;
    }
    return success;
}

extern "C" void NSBXX_DetachTextureImageFromModel(Nitro3DModel* model)
{
    Nitro3DModelMaterials* materialData = model->GetMaterialData();
    Nitro3DNameList* pairingList = (Nitro3DNameList*)((intptr_t)materialData + materialData->texturePairingsOffset_);

    for (unsigned int i = 0; i < pairingList->numEntries_; i++)
    {
        Nitro3DModelMaterials::Pair* pairing = pairingList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);
        if (pairing->flags_ & 1)
        {
            DetachTextureImageFromModelMaterials(materialData, pairing);
        }
    }
}

extern "C" void AttachTexturePaletteToModelMaterials(
    Nitro3DModelMaterials* materialData, Nitro3DModelMaterials::Pair* pairing,
    Nitro3DTextureSet* tex0, Nitro3DTextureSet::PaletteData* palette)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray_;
    
    unsigned short paletteVramOffset;
    unsigned short tex0vramOffset;
    
    tex0vramOffset = (uint16_t)tex0->block4VRAMLoadOffset_ & ~0xe0000000;
    paletteVramOffset = palette->offsetWithinBlock4_;
    
    if (!(palette->unk_2 & 1))
    {
        paletteVramOffset /= 2;
        tex0vramOffset /= 2;
    }

    for (unsigned int i = 0; i < pairing->arraySize_; i++)
    {
        unsigned int materialIdx = indexArray[i];
        Nitro3DMaterial* material = materialData->GetMaterialByIndex(materialIdx);
        material->texturePaletteVRAMOffset_ = paletteVramOffset + tex0vramOffset;
    }
    pairing->flags_ |= 1;
}

bool NSBXX_AttachTexturePaletteToModel(Nitro3DModel* model, Nitro3DTextureSet* tex0)
{
    Nitro3DModelMaterials* materialData = model->GetMaterialData();
    Nitro3DNameList* palettePairingsList = (Nitro3DNameList*)((const char*)materialData + materialData->palettePairingsOffset_);
    unsigned int i;
    bool success = true;

    for (i = 0; i < palettePairingsList->numEntries_; i++)
    {
        const char* paletteName = palettePairingsList->GetNameByIndex(i);

        Nitro3DTextureSet::PaletteData* palette;
        if (tex0 != NULL && tex0->paletteListOffset_ != 0)
            palette = (Nitro3DTextureSet::PaletteData*)NSBXXNameList_Search((Nitro3DNameList*)((const char*)tex0 + tex0->paletteListOffset_), paletteName);
        else
            palette = NULL;

        if (palette != NULL)
        {
            Nitro3DModelMaterials::Pair* pairing = palettePairingsList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);
            if ((pairing->flags_ & 1) == 0)
                AttachTexturePaletteToModelMaterials(materialData, pairing, tex0, palette);
        }
        else
            success = false;
    }
    return success;
}

extern "C" void NSBXX_DetachTexturePaletteFromModel(Nitro3DModel* model)
{
    Nitro3DModelMaterials* materialData = model->GetMaterialData();
    Nitro3DNameList* pairingList = (Nitro3DNameList*)((const char*)materialData + materialData->palettePairingsOffset_);

    unsigned int i = 0;
    for (unsigned int i = 0; i < pairingList->numEntries_; i++)
    {
        Nitro3DModelMaterials::Pair* pairing = pairingList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);
        if (pairing->flags_ & 1)
            pairing->flags_ &= ~1;
    }
}

extern "C" int NSBXX_LinkTEX0ToMDL0(Nitro3DModelSet* mdl0, Nitro3DTextureSet* tex0)
{
    unsigned int i;
    int success = true;
    
    for (i = 0; i < mdl0->nameList_.numEntries_; i++)
    {
        Nitro3DModel* internalModel = mdl0->GetInternalModelByIndex(i);
        success &= NSBXX_AttachTextureImageToModel(internalModel, tex0);
        success &= NSBXX_AttachTexturePaletteToModel(internalModel, tex0);
    }
    return success;
}

extern "C" void NSBXX_UnlinkTEX0FromMDL0(Nitro3DModelSet* mdl0)
{
    for (unsigned int i = 0; i < mdl0->nameList_.numEntries_; i++)
    {
        Nitro3DModel* internalModel = mdl0->GetInternalModelByIndex(i);
        NSBXX_DetachTextureImageFromModel(internalModel);
        NSBXX_DetachTexturePaletteFromModel(internalModel);
    }
}