#include "Graphics/NSBXX/NSBXX.h"
#include "System/LoadToVRAM.h"
#include <globaldefs.h>
#include <asmhacks.h>

#pragma optimize_for_size off

int NSBXX_Tex_GetBlock1Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block1NumEightBytes_ << 3 : 0;
}

int NSBXX_Tex_GetBlock2Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block2NumEightBytes_ << 3 : 0;
}

void NSBXX_Tex_WriteImageVRAMOffsets(NSBXXTex* tex, int block1, int block2_3)
{
    if (block1 != 0)
        tex->block1VRAMLoadOffset_ = block1;
    if (block2_3 != 0)
        tex->block2Or3VRAMLoadOffset_ = block2_3;
}

void NSBXX_Tex_LoadImageToVRAM(NSBXXTex* tex, bool needsMapping)
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

int NSBXX_Tex_GetBlock4Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block4NumEightBytes_ << 3 : 0;
}

void NSBXX_Tex_WritePaletteVRAMOffset(NSBXXTex* tex, int offset)
{
    tex->block4VRAMLoadOffset_ = offset;
}

void NSBXX_Tex_LoadPaletteToVRAM(NSBXXTex* tex, bool needsMapping)
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
    NSBXXModelMaterialData* materialData, NSBXXMaterialPairing* pairing,
    NSBXXTex* tex0, NSBXXTexTexture* texture)
{
    unsigned int pairedMaterialCounter = 0;
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray_;
    int trueTextureWidth;
    int trueTextureHeight;
    
    unsigned int tex0VramLoadOffset;
    if ((texture->paramTEXIMAGE_PARAMS_ & 0x1c000000) != 0x14000000)
        tex0VramLoadOffset = (unsigned short)tex0->block1VRAMLoadOffset_ & ~0xe0000000;
    else
        tex0VramLoadOffset = (unsigned short)tex0->block2Or3VRAMLoadOffset_ & ~0xe0000000;

    if (pairing->arraySize_ > UNSIGNED_ZERO())
    {
        do
        {
            unsigned int materialIdx = indexArray[pairedMaterialCounter];
            NSBXXMaterial* material = materialData->GetMaterialByIndex(materialIdx);
            material->paramTEXIMAGE_PARAMS_ |= (texture->paramTEXIMAGE_PARAMS_ + tex0VramLoadOffset);
            trueTextureWidth = texture->unk_4 & 0x7ff;
            trueTextureHeight = (texture->unk_4 & 0x3ff800) >> 11;

            if (trueTextureWidth == material->width_)
                material->xScale_ = 1 << 12;
            else
                material->xScale_ = fix32_Divide(trueTextureWidth << 12, material->width_ << 12);

            if (trueTextureHeight == material->height_)
                material->yScale_ = 1 << 12;
            else
                material->yScale_ = fix32_Divide(trueTextureHeight << 12, material->height_ << 12);

            pairedMaterialCounter++;
        } while (pairedMaterialCounter < pairing->arraySize_);
    }
    
    pairing->flags_ |= 1;
}

extern "C" void DetachTextureImageFromModelMaterials(NSBXXModelMaterialData* materialData, NSBXXMaterialPairing* pairing)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray_;
    unsigned int counter = 0;
    if (pairing->arraySize_ > UNSIGNED_ZERO())
    {
        do
        {
            NSBXXMaterial* material = materialData->GetMaterialByIndex(indexArray[counter]);
            counter++;
            // keep only bits 30-31 and 16,17,18,19
            material->paramTEXIMAGE_PARAMS_ &= 0xc00f0000;
            material->xScale_ = 1 << 12;
            material->yScale_ = 1 << 12;
        } while (counter < pairing->arraySize_);
    }
    pairing->flags_ &= ~1;
}

extern "C" bool NSBXX_AttachTextureImageToModel(NSBXXInternalModel* model, NSBXXTex* tex0)
{
    NSBXXNameList* imagePairingsList;
    unsigned int pairCounter;
    bool success = true;
    NSBXXModelMaterialData* materialData;
    if (model != NULL && model->materialsOffset_ != 0)
        materialData = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materialData = NULL;
    
    pairCounter = 0;
    imagePairingsList = (NSBXXNameList*)((intptr_t)materialData + materialData->texturePairingsOffset_);
    if (imagePairingsList->numEntries_ > UNSIGNED_ZERO())
    {
        int nameOffsetInBuffer = 0;
        do
        {
            const char* textureName = imagePairingsList->GetNameByIndexAndOffset(pairCounter, nameOffsetInBuffer);
            NSBXXTexTexture* texture;
            if (tex0 != NULL)
                texture = (NSBXXTexTexture*)NSBXXNameList_Search(&tex0->textureList_, textureName);
            else
                texture = NULL;

            if (texture != NULL)
            {
                NSBXXMaterialPairing* pairing = imagePairingsList->GetEntryByIndex<NSBXXMaterialPairing>(pairCounter);

                if ((pairing->flags_ & 1) == 0)
                    AttachTextureImageToModelMaterials(materialData, pairing, tex0, texture);
            }
            else
                success = false;

            pairCounter++;
            nameOffsetInBuffer += 16;
        } while (pairCounter < imagePairingsList->numEntries_);
    }
    return success;
}

extern "C" void NSBXX_DetachTextureImageFromModel(NSBXXInternalModel* model)
{
    NSBXXNameList* pairingList;
    unsigned int pairingCounter;

    NSBXXModelMaterialData* materialData;
    if (model != NULL && model->materialsOffset_ != 0)
        materialData = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materialData = NULL;    
    
    pairingList = (NSBXXNameList*)((intptr_t)materialData + materialData->texturePairingsOffset_);

    pairingCounter = 0;
    if (pairingList->numEntries_ > UNSIGNED_ZERO())
    {
        do
        {
            NSBXXMaterialPairing* pairing = pairingList->GetEntryByIndex<NSBXXMaterialPairing>(pairingCounter);
            if (pairing->flags_ & 1)
            {
                DetachTextureImageFromModelMaterials(materialData, pairing);
            }
            pairingCounter++;
        } while (pairingCounter < pairingList->numEntries_);
    }
}

// not a match
extern "C" void AttachTexturePaletteToModelMaterials(
    NSBXXModelMaterialData* materialData, NSBXXMaterialPairing* pairing,
    NSBXXTex* tex0, NSBXXTexPalette* palette)
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

    unsigned int pairedMaterialCounter = 0;
    if (pairing->arraySize_ > UNSIGNED_ZERO())
    { 
        do 
        {
            unsigned int materialIdx = indexArray[pairedMaterialCounter];
            NSBXXMaterial* material = materialData->GetMaterialByIndex(materialIdx);
            material->texturePaletteVRAMOffset_ = paletteVramOffset + tex0vramOffset;
            pairedMaterialCounter++;
        } while (pairedMaterialCounter < pairing->arraySize_);
    }
    pairing->flags_ |= 1;
}

bool NSBXX_AttachTexturePaletteToModel(NSBXXInternalModel* model, NSBXXTex* tex0)
{
    NSBXXNameList* palettePairingsList;
    unsigned int pairCounter;
    bool success = true;
    NSBXXModelMaterialData* materialData;
    if (model != NULL && model->materialsOffset_ != 0)
        materialData = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materialData = NULL;

    pairCounter = 0;
    palettePairingsList = (NSBXXNameList*)((intptr_t)materialData + materialData->palettePairingsOffset_);
    if (palettePairingsList->numEntries_ > UNSIGNED_ZERO())
    {
        int nameOffsetInBuffer = 0;
        do
        {
            const char* paletteName = palettePairingsList->GetNameByIndexAndOffset(pairCounter, nameOffsetInBuffer);

            NSBXXTexPalette* palette;
            if (tex0 != NULL && tex0->paletteListOffset_ != 0)
            {
                palette = (NSBXXTexPalette*)NSBXXNameList_Search((NSBXXNameList*)((intptr_t)tex0 + tex0->paletteListOffset_), paletteName);
            }
            else
            {
                palette = NULL;
            }

            if (palette != NULL)
            {
                NSBXXMaterialPairing* pairing = palettePairingsList->GetEntryByIndex<NSBXXMaterialPairing>(pairCounter);
                if ((pairing->flags_ & 1) == 0)
                    AttachTexturePaletteToModelMaterials(materialData, pairing, tex0, palette);
            }
            else
                success = false;

            pairCounter++;
            nameOffsetInBuffer += 16;
        } while (pairCounter < palettePairingsList->numEntries_);
    }
    return success;
}

extern "C" void NSBXX_DetachTexturePaletteFromModel(NSBXXInternalModel* model)
{
    NSBXXNameList* pairingList;
    unsigned int pairingCounter;

    NSBXXModelMaterialData* materialData;
    if (model != NULL && model->materialsOffset_ != 0)
        materialData = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
    else
        materialData = NULL;    
    
    pairingList = (NSBXXNameList*)((intptr_t)materialData + materialData->palettePairingsOffset_);

    pairingCounter = 0;
    if (pairingList->numEntries_ > UNSIGNED_ZERO())
    {
        do
        {
            NSBXXMaterialPairing* pairing = pairingList->GetEntryByIndex<NSBXXMaterialPairing>(pairingCounter);
            if (pairing->flags_ & 1)
            {
                pairing->flags_ &= ~1;
            }
            pairingCounter++;
        } while (pairingCounter < pairingList->numEntries_);
    }
}

extern "C" int NSBXX_LinkTEX0ToMDL0(NSBXXMdl* mdl0, NSBXXTex* tex0)
{
    unsigned int modelCounter;
    int success = true;
    modelCounter = 0;
    if (mdl0->nameList_.numEntries_ > UNSIGNED_ZERO())
    {
        do
        {
            NSBXXInternalModel* internalModel = mdl0->GetInternalModelByIndex(modelCounter);
            success &= NSBXX_AttachTextureImageToModel(internalModel, tex0);
            success &= NSBXX_AttachTexturePaletteToModel(internalModel, tex0);
            modelCounter++;
        } while (modelCounter < mdl0->nameList_.numEntries_);
    }
    return success;
}

extern "C" void NSBXX_UnlinkTEX0FromMDL0(NSBXXMdl* mdl0)
{
    unsigned int modelCounter = 0;
    if (mdl0->nameList_.numEntries_ > UNSIGNED_ZERO())
    {
        do
        {
            NSBXXInternalModel* internalModel = mdl0->GetInternalModelByIndex(modelCounter);
            NSBXX_DetachTextureImageFromModel(internalModel);
            NSBXX_DetachTexturePaletteFromModel(internalModel);
            modelCounter++;
        } while (modelCounter < mdl0->nameList_.numEntries_);
    }
}