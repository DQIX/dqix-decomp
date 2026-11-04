#include "NitroSDK/Graphics3D/Graphics3D.h"
#include "NitroSDK/LoadToVRAM.h"
#include <globaldefs.h>
#include <asmhacks.h>

int NSBXX_Tex_GetBlock1Length(Nitro3DTextureSet* tex)
{
    return (tex != NULL) ? tex->block1NumEightBytes << 3 : 0;
}

int NSBXX_Tex_GetBlock2Length(Nitro3DTextureSet* tex)
{
    return (tex != NULL) ? tex->block2NumEightBytes << 3 : 0;
}

void NSBXX_Tex_WriteImageVRAMOffsets(Nitro3DTextureSet* tex, int block1, int block2_3)
{
    if (block1 != 0)
        tex->block1VRAMLoadOffset = block1;
    if (block2_3 != 0)
        tex->block2Or3VRAMLoadOffset = block2_3;
}

void NSBXX_Tex_LoadImageToVRAM(Nitro3DTextureSet* tex, bool needsMapping)
{
    if (needsMapping)
        MemoryMapTextureImage();
    unsigned int block1Length = tex->block1NumEightBytes << 3;
    if (block1Length != 0)
    {
        LoadToTextureImage((const void*)((intptr_t)tex + tex->block1Offset),
            (uint16_t)tex->block1VRAMLoadOffset << 3, block1Length);
        tex->maybeBlock1Flags_10 |= (1 << 0);
    }
    int block3SourceOffset;
    unsigned int block2Length = tex->block2NumEightBytes << 3;
    if (block2Length != 0)
    {
        block3SourceOffset = tex->block3Offset;
        unsigned int block2WriteOffset = (tex->block2Or3VRAMLoadOffset << 16) >> 13;
        int block2SourceOffset = tex->block2Offset;
        
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
        tex->maybeBlock23Flags_20 |= (1 << 0);
    }
    if (needsMapping)
        MemoryUnmapTextureImage();
}

int NSBXX_Tex_GetBlock4Length(Nitro3DTextureSet* tex)
{
    return (tex != NULL) ? tex->block4NumEightBytes << 3 : 0;
}

void NSBXX_Tex_WritePaletteVRAMOffset(Nitro3DTextureSet* tex, int offset)
{
    tex->block4VRAMLoadOffset = offset;
}

void NSBXX_Tex_LoadPaletteToVRAM(Nitro3DTextureSet* tex, bool needsMapping)
{
    if (needsMapping)
        MemoryMapTexturePalette();

    LoadToTexturePalette((const void*)((intptr_t)tex + tex->block4Offset),
        (uint16_t)tex->block4VRAMLoadOffset << 3,
        tex->block4NumEightBytes << 3);
    tex->maybeBlock4Flags_32 |= (1 << 0);

    if (needsMapping)
        MemoryUnmapTexturePalette();
}

extern "C" void AttachTextureImageToModelMaterials(
    Nitro3DModelMaterials* materialData, Nitro3DModelMaterials::Pair* pairing,
    Nitro3DTextureSet* tex0, Nitro3DTextureSet::ImageData* texture)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray;
    unsigned int tex0VramLoadOffset;

    if ((texture->paramTEXIMAGE_PARAMS & 0x1c000000) != 0x14000000)
        tex0VramLoadOffset = (unsigned short)tex0->block1VRAMLoadOffset & ~0xe0000000;
    else
        tex0VramLoadOffset = (unsigned short)tex0->block2Or3VRAMLoadOffset & ~0xe0000000;

    
    for (unsigned int i = 0; i < pairing->arraySize; i++)
    {
        Nitro3DMaterial* material = materialData->GetMaterialByIndex(indexArray[i]);
        material->paramTEXIMAGE_PARAMS |= (texture->paramTEXIMAGE_PARAMS + tex0VramLoadOffset);
        
        int trueTextureWidth = texture->dimensions & 0x7ff;
        int trueTextureHeight = (texture->dimensions & 0x3ff800) >> 11;

        if (trueTextureWidth == material->width)
            material->xScale = 1 << 12;
        else
            material->xScale = fix32_Divide(trueTextureWidth << 12, material->width << 12);

        if (trueTextureHeight == material->height)
            material->yScale = 1 << 12;
        else
            material->yScale = fix32_Divide(trueTextureHeight << 12, material->height << 12);
    }
    
    pairing->flags |= 1;
}

extern "C" void DetachTextureImageFromModelMaterials(Nitro3DModelMaterials* materialData, Nitro3DModelMaterials::Pair* pairing)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray;
    for (unsigned int i = 0; i < pairing->arraySize; i++)
    {
        Nitro3DMaterial* material = materialData->GetMaterialByIndex(indexArray[i]);
        // keep only bits 30-31 and 16,17,18,19
        material->paramTEXIMAGE_PARAMS &= 0xc00f0000;
        material->xScale = 1 << 12;
        material->yScale = 1 << 12;
    }
    pairing->flags &= ~1;
}

extern "C" bool NSBXX_AttachTextureImageToModel(Nitro3DModel* model, Nitro3DTextureSet* tex0)
{
    Nitro3DModelMaterials* materialData = model->GetMaterialData();
    Nitro3DNameList* imagePairingsList = (Nitro3DNameList*)((const char*)materialData + materialData->texturePairingsOffset);
    
    unsigned int i;
    bool success = true;
    for (i = 0; i < imagePairingsList->numEntries; i++)
    {
        const char* textureName = imagePairingsList->GetNameByIndex(i);
        Nitro3DTextureSet::ImageData* texture = (tex0 != NULL) ?
            (Nitro3DTextureSet::ImageData*)NSBXXNameList_Search(&tex0->textureList, textureName) : NULL;
        if (texture != NULL)
        {
            Nitro3DModelMaterials::Pair* pairing = imagePairingsList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);

            if (!(pairing->flags & 1))
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
    Nitro3DNameList* pairingList = (Nitro3DNameList*)((intptr_t)materialData + materialData->texturePairingsOffset);

    for (unsigned int i = 0; i < pairingList->numEntries; i++)
    {
        Nitro3DModelMaterials::Pair* pairing = pairingList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);
        if (pairing->flags & 1)
        {
            DetachTextureImageFromModelMaterials(materialData, pairing);
        }
    }
}

extern "C" void AttachTexturePaletteToModelMaterials(
    Nitro3DModelMaterials* materialData, Nitro3DModelMaterials::Pair* pairing,
    Nitro3DTextureSet* tex0, Nitro3DTextureSet::PaletteData* palette)
{
    uint8_t* indexArray = (uint8_t*)materialData + pairing->offsetToIndexArray;
    
    unsigned short paletteVramOffset;
    unsigned short tex0vramOffset;
    
    tex0vramOffset = (uint16_t)tex0->block4VRAMLoadOffset & ~0xe0000000;
    paletteVramOffset = palette->offsetWithinBlock4;
    
    if (!(palette->unknown_2 & 1))
    {
        paletteVramOffset /= 2;
        tex0vramOffset /= 2;
    }

    for (unsigned int i = 0; i < pairing->arraySize; i++)
    {
        unsigned int materialIdx = indexArray[i];
        Nitro3DMaterial* material = materialData->GetMaterialByIndex(materialIdx);
        material->texturePaletteVRAMOffset = paletteVramOffset + tex0vramOffset;
    }
    pairing->flags |= 1;
}

bool NSBXX_AttachTexturePaletteToModel(Nitro3DModel* model, Nitro3DTextureSet* tex0)
{
    Nitro3DModelMaterials* materialData = model->GetMaterialData();
    Nitro3DNameList* palettePairingsList = (Nitro3DNameList*)((const char*)materialData + materialData->palettePairingsOffset);
    unsigned int i;
    bool success = true;

    for (i = 0; i < palettePairingsList->numEntries; i++)
    {
        const char* paletteName = palettePairingsList->GetNameByIndex(i);

        Nitro3DTextureSet::PaletteData* palette;
        if (tex0 != NULL && tex0->paletteListOffset != 0)
            palette = (Nitro3DTextureSet::PaletteData*)NSBXXNameList_Search((Nitro3DNameList*)((const char*)tex0 + tex0->paletteListOffset), paletteName);
        else
            palette = NULL;

        if (palette != NULL)
        {
            Nitro3DModelMaterials::Pair* pairing = palettePairingsList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);
            if ((pairing->flags & 1) == 0)
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
    Nitro3DNameList* pairingList = (Nitro3DNameList*)((const char*)materialData + materialData->palettePairingsOffset);

    unsigned int i = 0;
    for (unsigned int i = 0; i < pairingList->numEntries; i++)
    {
        Nitro3DModelMaterials::Pair* pairing = pairingList->GetEntryByIndex<Nitro3DModelMaterials::Pair>(i);
        if (pairing->flags & 1)
            pairing->flags &= ~1;
    }
}

extern "C" int NSBXX_LinkTEX0ToMDL0(Nitro3DModelSet* mdl0, Nitro3DTextureSet* tex0)
{
    unsigned int i;
    int success = true;
    
    for (i = 0; i < mdl0->nameList.numEntries; i++)
    {
        Nitro3DModel* internalModel = mdl0->GetInternalModelByIndex(i);
        success &= NSBXX_AttachTextureImageToModel(internalModel, tex0);
        success &= NSBXX_AttachTexturePaletteToModel(internalModel, tex0);
    }
    return success;
}

extern "C" void NSBXX_UnlinkTEX0FromMDL0(Nitro3DModelSet* mdl0)
{
    for (unsigned int i = 0; i < mdl0->nameList.numEntries; i++)
    {
        Nitro3DModel* internalModel = mdl0->GetInternalModelByIndex(i);
        NSBXX_DetachTextureImageFromModel(internalModel);
        NSBXX_DetachTexturePaletteFromModel(internalModel);
    }
}