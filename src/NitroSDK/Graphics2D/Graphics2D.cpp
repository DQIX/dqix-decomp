#include "NitroSDK/Graphics2D/Graphics2D.h"
#include "NitroSDK/HWRegisters.h"
#include "NitroSDK/Cache.h"
#include "NitroSDK/LoadToVRAM.h"
#include <globaldefs.h>

#define SIGNATURE_ABNK 0x41424e4b
#define SIGNATURE_CEBK 0x4345424b
#define SIGNATURE_CHAR 0x43484152
#define SIGNATURE_PLTT 0x504c5454
#define SIGNATURE_PCMP 0x50434d50

template<class T>
static inline T* FixPointerOffsetRight(T** pPtr, void* base)
{
    return (*pPtr = (T*)((intptr_t)*pPtr + (intptr_t)base));
}

template<class T>
static inline T* FixPointerOffsetLeft(T** pPtr, void* base)
{
    return (*pPtr = (T*)((intptr_t)base + (intptr_t)*pPtr));
}

static inline int Log2Discrete(int x)
{
    switch (x)
    {
    case 1: return 0;
    case 2: return 1;
    case 4: return 2;
    case 8: return 3;
    case 16: return 4;
    case 32: return 5;
    }
    return 0;
}

static void InternalPrepareCellBankSubstruct14(Nitro2DCellBank::Substruct14::Interior* ptr);
static void PrepareABNK(Nitro2DAnimationBank* ptr);
static void PrepareCEBK(Nitro2DCellBank* ptr);
static void PrepareCHAR(Nitro2DCharacter* ptr);
static void PreparePLTT(Nitro2DPalette* ptr);
static void PreparePCMP(Nitro2DPaletteCMP* ptr);

static CBool InternalGetAnimationBank(void* fileData, Nitro2DAnimationBank** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_ABNK);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PrepareABNK((Nitro2DAnimationBank*)fileHeader->GetInterior());
    *output = (Nitro2DAnimationBank*)fileHeader->GetInterior();
    return true;
}

CBool Nitro2D_GetAnimationBank(void* fileData, Nitro2DAnimationBank** output)
{
    return InternalGetAnimationBank(fileData, output);
}

static void PrepareABNK(Nitro2DAnimationBank* bank)
{
    FixPointerOffsetRight(&bank->pAnimBlocks, bank);
    FixPointerOffsetRight(&bank->pFrameBlocks, bank);
    FixPointerOffsetRight(&bank->pFrameData, bank);

    unsigned short blockIndex;
    unsigned short frameIndex;
    uint16_t* frameData = bank->pFrameData;
    Nitro2DAnimationBank::AnimationBlock* animBlocks = bank->pAnimBlocks;
    Nitro2DAnimationBank::FrameBlock* frameBlocks = bank->pFrameBlocks;

    for (blockIndex = 0; bank->numAnimBlocks > blockIndex; blockIndex++)
    {
        FixPointerOffsetLeft(&animBlocks[blockIndex].pFrames, frameBlocks);
        for (frameIndex = 0; animBlocks[blockIndex].numFrames > frameIndex; frameIndex++)
            FixPointerOffsetLeft(&animBlocks[blockIndex].pFrames[frameIndex].metatileIndices, frameData);
    }

    if (bank->unknown_14 == NULL)
        return;

    FixPointerOffsetRight(&bank->unknown_14, bank);
    Nitro2DAnimationBank::Subsubstruct* subsub = &bank->unknown_14->unknown_8;
    FixPointerOffsetRight(&subsub->unknown_4, subsub);
    for (unsigned int i = 0; i < subsub->unknown_0; i++)
    {
        unsigned int j;
        Nitro2DAnimationBank::Subsubsub* sss = &subsub->unknown_4[i];
        FixPointerOffsetRight(&sss->unknown_4, subsub);
        FixPointerOffsetRight(&sss->unknown_8, subsub);
        for (j = 0; j < sss->unknown_0; j++)
            FixPointerOffsetRight(&sss->unknown_8[j], subsub);
    }
}

Nitro2DAnimationBank::AnimationBlock* Nitro2D_AnimationBankGetAnimationBlock(Nitro2DAnimationBank* bank, unsigned int idx)
{
    if (bank->numAnimBlocks > idx)
        return bank->pAnimBlocks + idx;
    return NULL;
}

static OAMEntry* GetCellBankOBJAttributes(Nitro2DCellBank* bank)
{
    // OBJ attributes begin at the end of the metatile table
    if (bank->hasExtraMetatileData & 1)
        return (OAMEntry*)((Nitro2DCellBank::MetatileEXT*)bank->pMetatiles + bank->numMetatiles);
    else
        return (OAMEntry*)(bank->pMetatiles + bank->numMetatiles);
}

static void PrepareCellBankSubstruct14(Nitro2DCellBank::Substruct14* subs)
{
    InternalPrepareCellBankSubstruct14(&subs->interior);
}

CBool Nitro2D_GetCellBank(void* fileData, Nitro2DCellBank** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_CEBK);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PrepareCEBK((Nitro2DCellBank*)fileHeader->GetInterior());
    *output = (Nitro2DCellBank*)fileHeader->GetInterior();
    return true;
}

Nitro2DCellBank::Metatile* Nitro2D_GetCellBankMetatile(Nitro2DCellBank* bank, unsigned short idx)
{
    if (idx >= bank->numMetatiles)
        return NULL;
    if (bank->hasExtraMetatileData & 1)
        return (Nitro2DCellBank::Metatile*)((Nitro2DCellBank::MetatileEXT*)bank->pMetatiles + idx);
    else
        return bank->pMetatiles + idx;
}

static void PrepareCEBK(Nitro2DCellBank* bank)
{
    FixPointerOffsetRight(&bank->pMetatiles, bank);
    OAMEntry* oam = GetCellBankOBJAttributes(bank);

    for (unsigned short i = 0; i < bank->numMetatiles; i++)
    {
        Nitro2DCellBank::Metatile* metatile = Nitro2D_GetCellBankMetatile(bank, i);
        FixPointerOffsetRight(&metatile->pAttributes, oam);
    }

    if (bank->unknown_c != NULL)
    {
        Nitro2DCellBank::SubstructC* subc = FixPointerOffsetRight(&bank->unknown_c, bank);
        FixPointerOffsetRight(&subc->unknown_4, subc);
        bank->unknown_c = subc; // why write this twice?
    }

    if (bank->unknown_14 != NULL)
    {
        FixPointerOffsetRight(&bank->unknown_14, bank);
        PrepareCellBankSubstruct14(bank->unknown_14);
    }
}

CBool Nitro2D_GetCharacter(void* fileData, Nitro2DCharacter** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_CHAR);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PrepareCHAR((Nitro2DCharacter*)fileHeader->GetInterior());
    *output = (Nitro2DCharacter*)fileHeader->GetInterior();
    return true;
}

static void PrepareCHAR(Nitro2DCharacter* character)
{
    FixPointerOffsetRight(&character->pTileData, character);
}

CBool Nitro2D_GetPalette(void* fileData, Nitro2DPalette** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_PLTT);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PreparePLTT((Nitro2DPalette*)fileHeader->GetInterior());
    *output = (Nitro2DPalette*)fileHeader->GetInterior();
    return true;
}

CBool Nitro2D_GetPaletteCMP(void* fileData, Nitro2DPaletteCMP** output)
{
    Nitro2DInnerFileHeader* fileHeader = Nitro2D_GetInnerFileBySignature(fileData, SIGNATURE_PCMP);
    if (fileHeader == NULL)
    {
        *output = NULL;
        return false;
    }
    PreparePCMP((Nitro2DPaletteCMP*)fileHeader->GetInterior());
    *output = (Nitro2DPaletteCMP*)fileHeader->GetInterior();
    return true;
}

static void PreparePCMP(Nitro2DPaletteCMP* pcmp)
{
    FixPointerOffsetRight(&pcmp->pPaletteIDs, pcmp);
}

static void PreparePLTT(Nitro2DPalette* palette)
{
    FixPointerOffsetRight(&palette->pPaletteData, palette);
}

Nitro2DInnerFileHeader* Nitro2D_GetInnerFileBySignature(void* fileData, uint32_t signature)
{
    Nitro2DFileHeader* header = (Nitro2DFileHeader*)fileData;
    Nitro2DInnerFileHeader* innerFile = header->GetFirstSubfile();
    for (unsigned short i = 0; i < header->numSubfiles; i++)
    {
        if (innerFile->signature == signature)
            return innerFile;
        innerFile = innerFile->Next();
    }
    return NULL;
}

static void InternalPrepareCellBankSubstruct14(Nitro2DCellBank::Substruct14::Interior* subs)
{
    FixPointerOffsetRight(&subs->pointers_4, subs);
    for (unsigned short i = 0; i < subs->numPointers_0; i++)
    {
        FixPointerOffsetLeft(&subs->pointers_4[i], subs);
    }
}

void Nitro2D_ClearCharacterLoadDesc(Nitro2DCharacterLoadDesc* desc)
{
    for (int i = 0; i < 3; i++)
        desc->loadOffsets[i] = -1;
}

static void StoreCharacterVRAMLoadOffset(Nitro2DCharacterLoadDesc* output, int vramType, unsigned int offset)
{
    output->loadOffsets[vramType] = offset;
}

void Nitro2D_ClearPaletteLoadDesc(Nitro2DPaletteLoadDesc* desc)
{
    for (int i = 0; i < 3; i++)
        desc->loadOffsets[i] = -1;
}

static void StorePaletteVRAMLoadOffset(Nitro2DPaletteLoadDesc* output, int screen, unsigned int offset)
{
    output->loadOffsets[screen] = offset;
}

void Nitro2D_LoadCharacterToVRAM(Nitro2DCharacter* character, unsigned int vramOffset, int vramType, Nitro2DCharacterLoadDesc* output)
{
    unsigned int objTileMapping = character->objTileMappingFlags;
    switch (vramType)
    {
    case 0:
        break;
    case 1:
        DISPCNT = (DISPCNT & 0xffcfffef) | objTileMapping;
        break;
    case 2:
        DISPCNTSUB = (DISPCNTSUB & 0xffcfffef) | objTileMapping;
        break;
    }
    CleanInvalidateCacheRange(character->pTileData, character->tileDataSize);
    switch (vramType)
    {
    case 0:
        MemoryMapTextureImage();
        LoadToTextureImage(character->pTileData, vramOffset, character->tileDataSize);
        MemoryUnmapTextureImage();
        break;
    case 1:
        LoadToMainObjVRAM(character->pTileData, vramOffset, character->tileDataSize);
        break;
    case 2:
        LoadToSubObjVRAM(character->pTileData, vramOffset, character->tileDataSize);
        break;
    }
    if (character->objTileMappingFlags == 0) // 2D tile mapping
    {
        output->unknown_c = Log2Discrete(character->unknown_2);
        output->unknown_10 = Log2Discrete(character->unknown_0);
    }
    else
    {
        output->unknown_c = character->unknown_2;
        output->unknown_10 = character->unknown_0;
    }
    output->unknown_14 = character->unknown_4;
    output->unknown_18 = 0;
    output->unknown_1c = 1;
    output->objTileMapping = character->objTileMappingFlags;
    StoreCharacterVRAMLoadOffset(output, vramType, vramOffset);
}

void Nitro2D_LoadCharacterToVRAM2(Nitro2DCharacter* character, unsigned int vramOffset, int vramType, Nitro2DCharacterLoadDesc* output)
{
    unsigned int objTileMapping = character->objTileMappingFlags;
    switch (vramType)
    {
    case 0:
        break;
    case 1:
        DISPCNT = (DISPCNT & 0xffcfffef) | objTileMapping;
        break;
    case 2:
        DISPCNTSUB = (DISPCNTSUB & 0xffcfffef) | objTileMapping;
        break;
    }
    CleanInvalidateCacheRange(character->pTileData, character->tileDataSize);
    switch (vramType)
    {
    case 0:
        MemoryMapTextureImage();
        LoadToTextureImage(character->pTileData, vramOffset, character->tileDataSize);
        MemoryUnmapTextureImage();
        break;
    case 1:
        LoadToMainObjVRAM(character->pTileData, vramOffset, character->tileDataSize);
        break;
    case 2:
        LoadToSubObjVRAM(character->pTileData, vramOffset, character->tileDataSize);
        break;
    }
    if (character->objTileMappingFlags == 0) // 2D tile mapping
    {
        output->unknown_c = Log2Discrete(character->unknown_2);
        output->unknown_10 = Log2Discrete(character->unknown_0);
    }
    else
    {
        output->unknown_c = character->unknown_2;
        output->unknown_10 = character->unknown_0;
    }
    output->unknown_14 = character->unknown_4;
    output->unknown_18 = 0;
    output->unknown_1c = 1;
    output->objTileMapping = character->objTileMappingFlags;
    StoreCharacterVRAMLoadOffset(output, vramType, vramOffset);
}

void Nitro2D_LoadPaletteToVRAM(Nitro2DPalette* palette, unsigned int vramOffset, int vramType, Nitro2DPaletteLoadDesc* output)
{
    uint8_t* paletteData = palette->pPaletteData;
    unsigned int dataLength = palette->paletteDataSize;
    CleanInvalidateCacheRange(paletteData, dataLength);
    switch (vramType)
    {
    case 1:
        if (palette->useExtendedPalette)
        {
            MemoryMapMainObjExtendedPalette();
            LoadToMainObjExtendedPalette(paletteData, vramOffset, dataLength);
            MemoryUnmapMainObjExtendedPalette();
        }
        else
        {
            LoadToMainObjStandardPalette(paletteData, vramOffset, dataLength);
        }
        break;
    case 2:
        if (palette->useExtendedPalette)
        {
            MemoryMapSubObjExtendedPalette();
            LoadToSubObjExtendedPalette(paletteData, vramOffset, dataLength);
            MemoryUnmapSubObjExtendedPalette();
        }
        else
        {
            LoadToSubObjStandardPalette(paletteData, vramOffset, dataLength);
        }
        break;
    case 0:
        MemoryMapTexturePalette();
        LoadToTexturePalette(paletteData, vramOffset, dataLength);
        MemoryUnmapTexturePalette();
        break;
    }
    output->textureFormat = palette->textureFormat;
    output->useExtendedPalette = palette->useExtendedPalette;
    StorePaletteVRAMLoadOffset(output, vramType, vramOffset);
}

void Nitro2D_LoadPaletteWithCMPToVRAM(Nitro2DPalette* palette, Nitro2DPaletteCMP* paletteCMP,
    unsigned int vramOffset, int vramType, Nitro2DPaletteLoadDesc* output)
{
    unsigned int loadSize = (palette->textureFormat == 3) ? 0x20 : 0x200;

    unsigned short numPalettes = paletteCMP->numPalettes;
    unsigned int paletteSourceOffset;
    uint8_t* paletteData;
    unsigned int dstOffset;
    for (unsigned short i = 0; i < numPalettes; i++)
    {
        paletteData = palette->pPaletteData;
        dstOffset = loadSize * paletteCMP->pPaletteIDs[i];
        paletteSourceOffset = loadSize * i;
        CleanInvalidateCacheRange(paletteData, palette->paletteDataSize);
        switch (vramType)
        {
        case 1:
            if (palette->useExtendedPalette)
            {
                MemoryMapMainObjExtendedPalette();
                LoadToMainObjExtendedPalette(paletteData + paletteSourceOffset,
                    vramOffset + dstOffset, loadSize);
                MemoryUnmapMainObjExtendedPalette();
            }
            else
            {
                LoadToMainObjStandardPalette(paletteData + paletteSourceOffset,
                    vramOffset + dstOffset, loadSize);
            }
            break;
        case 2:
            if (palette->useExtendedPalette)
            {
                MemoryMapSubObjExtendedPalette();
                LoadToSubObjExtendedPalette(paletteData + paletteSourceOffset,
                    vramOffset + dstOffset, loadSize);
                MemoryUnmapSubObjExtendedPalette();
            }
            else
            {
                LoadToSubObjStandardPalette(paletteData + paletteSourceOffset,
                    vramOffset + dstOffset, loadSize);
            }
            break;
        case 0:
            MemoryMapTexturePalette();
            LoadToTexturePalette(paletteData + paletteSourceOffset,
                vramOffset + dstOffset, loadSize);
            MemoryUnmapTexturePalette();
            break;
        }
    }
    output->textureFormat = palette->textureFormat;
    output->useExtendedPalette = palette->useExtendedPalette;
    StorePaletteVRAMLoadOffset(output, vramType, vramOffset);
}

static inline void TransformVectorByMatrix(const Matrix2x2* mat, const Vector2fix* in, Vector2fix* out)
{
    Vector2fix scratch;
    Vector2fix* writePtr = out;
    if (out == in)
        writePtr = &scratch;
    // actually important to add 0x800 two times
    writePtr->x = (((int64_t)mat->entriesL[0] * in->x + 0x800) + ((int64_t)mat->entriesL[2] * in->y + 0x800)) >> 12;
    writePtr->y = (((int64_t)mat->entriesL[1] * in->x + 0x800) + ((int64_t)mat->entriesL[3] * in->y + 0x800)) >> 12;
    if (writePtr == &scratch)
        *out = scratch;
}

static inline int GetObjHeight(int spaceSize)
{
    static const unsigned short heightLookup[3][4] = 
    {
        { 8, 16, 32, 64 },
        { 8, 8, 16, 32 },
        { 16, 32, 32, 64 }
    };
    return heightLookup[(spaceSize & 0xc000) >> 14][(spaceSize & 0xc0000000) >> 30];
}

static inline int GetObjWidth(int spaceSize)
{
    static const unsigned short widthLookup[3][4] = 
    {
        { 8, 16, 32, 64 },
        { 16, 32, 32, 64 },
        { 8, 8, 16, 32 },
    };
    return widthLookup[(spaceSize & 0xc000) >> 14][(spaceSize & 0xc0000000) >> 30];
}

int Nitro2D_DrawMetatile(OAMEntryEXT* targetOAMBuffer, unsigned short oamLength, 
    const Nitro2DCellBank::Metatile* metatile, const Matrix2x2* transform, const Vector2fix* position,
    unsigned short rotScaleIndex, CBool doubleSize)
{
    unsigned short writeCount = (oamLength < metatile->numObjs) ? oamLength : metatile->numObjs;

    for (unsigned short i = 0; i < writeCount; i++)
    {
        OAMEntryEXT* target = &targetOAMBuffer[i];
        target->base = metatile->pAttributes[i];
        if (transform == NULL && position == NULL)
            continue;
        Vector2fix finalPosition;
        short relX = (target->words01 & 0x1ff0000) >> 16;
        if (relX > 255)
            relX |= 0xff00; // wrap around, has effect of subtracting 512
        finalPosition.x = relX << 12;
        short relY = (target->words01 & 0xff);
        if (relY > 127)
            relY |= 0xff00; // wrap around, has effect of subtracting 256
        finalPosition.y = relY << 12;

        if (transform != NULL)
        {
            int transformFlags = target->words01 & 0x300;
            if (transformFlags != 0x100 && transformFlags != 0x300)
                transformFlags |= target->words01 & 0x30000000;
            if (transformFlags == 0x300) // double size
            {
                int shapeSize = (target->words01 & 0xc000c000);
                finalPosition.x += GetObjWidth(shapeSize) << 11;
                finalPosition.y += GetObjHeight(shapeSize) << 11;
            }

            int requestedTransformFlags = doubleSize ? 0x300 : 0x100;
            CBool doubleSizeAgain = (requestedTransformFlags == 0x300);

            TransformVectorByMatrix(transform, &finalPosition, &finalPosition);

            if (requestedTransformFlags != 0x100 && requestedTransformFlags != 0x300)
                target->words01 = (target->words01 & ~0x3e000300) | requestedTransformFlags;
            else // this case always runs
                target->words01 = (target->words01 & ~0x3e000300) | requestedTransformFlags | (rotScaleIndex << 25);

            int shapeSize = target->words01 & 0xc000c000;
            int halfWidth = GetObjWidth(shapeSize) >> 1;
            int halfHeight = GetObjHeight(shapeSize) >> 1;
            finalPosition.x += -(halfWidth << 12) + transform->entriesL[0] * halfWidth + transform->entriesL[2] * halfHeight;
            finalPosition.y += -(halfHeight << 12) + transform->entriesL[1] * halfWidth + transform->entriesL[3] * halfHeight;
            if (doubleSizeAgain)
            {
                finalPosition.x -= halfWidth << 12;
                finalPosition.y -= halfHeight << 12;
            }
        }

        if (position != NULL)
        {
            finalPosition.x += position->x;
            finalPosition.y += position->y;
        }

        int xInt = (finalPosition.x + 0x800) >> 12;
        int yInt = (finalPosition.y + 0x800) >> 12;
        target->words01 = (target->words01 & ~0x01ff00ff) | (yInt & 0xff) | ((xInt & 0x1ff) << 16);
    }
    return writeCount;
}