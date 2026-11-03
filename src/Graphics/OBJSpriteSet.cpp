#pragma ipa file

#include "Graphics/OBJSpriteSet.h"
#include "Resource/PAC.h"
#include <asmhacks.h>

// sin/cos table
extern const fix16_t data_020e9450[0x1000 * 2];

static inline fix16_t LookupSine(int idx)
{
    return data_020e9450[2 * idx];
}

static inline fix16_t LookupCosine(int idx)
{
    return data_020e9450[2 * idx + 1];
}

extern "C"
{
    // get obj manager instance
    void* func_0203bd08();
    // get sub-engine obj oam entries for staging
    OAMEntryEXT* func_0203be4c(void*);
    // get main engine obj oam entries for staging
    OAMEntryEXT* func_0203be40(void*);
}

extern char data_020f0504[]; // "RECN"
extern char data_020f0509[]; // "RNAN"
extern char data_020f050e[]; // "RGCN"
extern char data_020f0513[]; // "RLCN"

void OBJSpriteSet::Metatile::Reset()
{
    memset(&nitroData_0, 0, sizeof(Nitro2DCellBank::Metatile));
    oamEntries_8 = NULL;
    unknown_c = unknown_10 = 1 << 12;
    position_14.x = position_14.y = 0;
    normalizedAngle_1c = 0;
    unknown_1e = 0;
    rotScaleDataIndex_20 = 0;
    destinationIndex_22 = 0;
    useTransform_23 = 0;
    doubleSize_24 = false;
    paletteIndex_25 = -1;
    priority_26 = -1;
}

bool GetCharBlockFromNitroFile(Nitro2DCharacter** out, void* data, unsigned int length)
{
    if (out == NULL)
        return true;
    if (data == NULL)
        return true;
    if (length == 0)
        return true;
    if (!Nitro2D_GetCharacter(data, out))
        return true;
    return false;
}

void OBJSpriteSet::List::Reset()
{
    sprites_ = NULL;
    count_ = 0;
}

void OBJSpriteSet::List::Destroy()
{
    sprites_ = NULL;
    count_ = 0;
}

void OBJSpriteSet::List::Update(unsigned char id, int numTicks)
{
    unsigned short tickCount16 = numTicks;
    Sprite* sprite = GetSprite(id);
    if (sprite == NULL)
        return;
    if (!(sprite->flags_15 & 2) || sprite->animationType_0 == 0)
        return;
    sprite->frameTime_12 += tickCount16;
    AnimationFrame* frame = &sprite->frames_c[sprite->currentFrame_10];
    if (frame->duration_0 <= sprite->frameTime_12)
    {
        // don't simplify this to -=
        sprite->frameTime_12 = sprite->frameTime_12 - frame->duration_0;
        switch (sprite->animationType_0)
        {
        case 0:
            break;
        case 1:
        {
            sprite->currentFrame_10++;
            if (sprite->numFrames_a <= sprite->currentFrame_10)
            {
                sprite->currentFrame_10 = sprite->currentFrame_10 - 1;
                sprite->frameTime_12 = frame->duration_0;
                StopSpriteAnimation(sprite->id_8);
            }
            break;
        }
        case 2:
            sprite->currentFrame_10++;
            if (sprite->numFrames_a <= sprite->currentFrame_10)
                sprite->currentFrame_10 = 0;
            break;
        case 3:
            break;
        case 4:
            break;
        }
    }
}

void OBJSpriteSet::List::UpdateAll(int numTicks)
{
    for (unsigned short i = 0; i < count_; i++)
        Update(i, numTicks);
}

void OBJSpriteSet::List::PlaySpriteAnimation(unsigned int id)
{
    Sprite* sprite = GetSprite(id);
    if (sprite == NULL)
        return;
    if (sprite->flags_15 & 1)
    {
        sprite->currentFrame_10 = 0;
        sprite->frameTime_12 = 0;
    }
    sprite->flags_15 &= ~7;
    sprite->flags_15 |= 2;
}

void OBJSpriteSet::List::StopSpriteAnimation(unsigned int id)
{
    Sprite* sprite = GetSprite(id);
    if (sprite == NULL)
        return;
    sprite->flags_15 &= ~7;
    sprite->flags_15 |= 1;
}

OBJSpriteSet::Sprite* OBJSpriteSet::List::GetSprite(unsigned int id)
{
    if (sprites_ == NULL)
        return NULL;
    if (count_ == 0)
        return NULL;
    Sprite* ptr = NULL;
    for (unsigned short i = 0; i < count_; i++)
    {
        if (sprites_[i].id_8 == id)
        {
            ptr = &sprites_[i];
            break;
        }
    }
    return ptr;
}

void OBJSpriteSet::List::SetSpriteOBJIndex(unsigned short spriteID, unsigned char to)
{
    Sprite* sprite = GetSprite(spriteID);
    if (sprite != NULL)
        sprite->destinationIndex_14 = to;
}

void OBJSpriteSet::Reset()
{
    memset(&charLoad_0_, 0, sizeof(Nitro2DCharacterLoadDesc));
    memset(&paletteLoad_24_, 0, sizeof(Nitro2DPaletteLoadDesc));
    alignmentBoundaryType_38_ = 0;
    metatiles_40_ = NULL;
    charVRAMLoadOffset_44_ = 0;
    usedObjVRAMEnd_48_ = 0;
    numMetatiles_4c_ = 0;
    numMetatiles_4e_ = 0;
    sprites_3c_ = NULL;
    engine_50_ = true;
}

void OBJSpriteSet::Destroy()
{
}

bool OBJSpriteSet::LoadFromPAC(void* data, unsigned int length, SafeAllocator* alloc)
{
    if (length != 0)
    {
        int numFiles = GetNumFilesInPAC(data);
        for (int i = 0; i < numFiles; i++)
        {
            const char* innerName = NULL;
            unsigned int innerLength = 0;
            void* innerFile = GetFileInPACByIndex(data, i, &innerName, &innerLength);
            if (innerFile != NULL)
                LoadFromNitroFile(innerFile, innerLength, alloc);
        }
        return false; // wtf?
    }
    return true; // wtf?
}

bool OBJSpriteSet::LoadFromNitroFile(void* data, unsigned int length, SafeAllocator* alloc)
{
    char signatureString[5] = {0};
    memcpy(signatureString, data, 4);
    if (memcmp(signatureString, data_020f0504, 4) == 0)
    {
        if (LoadNCER(data, length, alloc))
            return true;
    }
    if (memcmp(signatureString, data_020f0509, 4) == 0)
    {
        if (LoadNANR(data, length, alloc))
            return true;
    }
    else if (memcmp(signatureString, data_020f050e, 4) == 0)
    {
        if (LoadNCGR(data, length))
            return true;
    }
    else if (memcmp(signatureString, data_020f0513, 4) == 0)
    {
        if (LoadNCLR(data, length))
            return true;
    }

    return false;
}

bool OBJSpriteSet::LoadNCER(void* argData, unsigned int length, SafeAllocator* alloc)
{
    void* data = argData;
    Nitro2DCellBank* cellBank;
    if (data == NULL)
        data = NULL; // yes, this is real code
    else if (length == 0)
        data = NULL;
    else
    {
        // ah yes. memcpy from data to data. my favorite
        memcpy(data, argData, length);
        if (!Nitro2D_GetCellBank(data, &cellBank))
            data = NULL;
    }

    if (data == NULL)
        return true;

    if (alloc != NULL && metatiles_40_ == NULL)
    {
        numMetatiles_4c_ = cellBank->numMetatiles;
        metatiles_40_ = (Metatile*)alloc->Allocate(numMetatiles_4c_ * sizeof(Metatile));
        for (unsigned short i = 0; i < numMetatiles_4c_; i++)
            metatiles_40_[i].Reset();
    }

    Metatile* pMetatiles = metatiles_40_;
    Metatile* localMetatile;
    unsigned short numMetatiles = numMetatiles_4c_;
    
    if (pMetatiles != NULL && numMetatiles != 0 && alloc != NULL)
    {
        unsigned short sourceNumMetatiles = cellBank->numMetatiles;
        Nitro2DCellBank::Metatile* nitroMetatile;
        unsigned short i;
        unsigned short objCounter = 0;
        for (i = 0; i < sourceNumMetatiles; i++)
        {
            if (numMetatiles <= i)
                break;
            nitroMetatile = Nitro2D_GetCellBankMetatile(cellBank, i);
            if (nitroMetatile == NULL)
                break;
            unsigned short thisNumObjs = nitroMetatile->numObjs;
            localMetatile = &pMetatiles[i];
            localMetatile->unknown_1e = i;
            localMetatile->destinationIndex_22 = objCounter;
            localMetatile->nitroData_0 = *nitroMetatile;
            localMetatile->nitroData_0.pAttributes = localMetatile->oamEntries_8 =
                (OAMEntry*)alloc->Allocate(thisNumObjs * sizeof(OAMEntry));
            for (unsigned short j = 0; j < nitroMetatile->numObjs; j++)
            {
                localMetatile->oamEntries_8[j] = nitroMetatile->pAttributes[j];
            }
            objCounter += nitroMetatile->numObjs;
        }
    }

    numMetatiles_4e_ = cellBank->numMetatiles;
    alignmentBoundaryType_38_ = cellBank->alignmentBoundaryType;

    Nitro2DCellBank::Metatile* lastNitroMT = Nitro2D_GetCellBankMetatile(cellBank, cellBank->numMetatiles - 1);
    if (lastNitroMT == NULL)
        return true;
    unsigned int logAlignmentBoundary = alignmentBoundaryType_38_ + 5;
    OAMEntry* lastOAM = &lastNitroMT->pAttributes[lastNitroMT->numObjs - 1];
    
    unsigned int objVRAMOffset = (lastOAM->word2 & 0x3ff) << logAlignmentBoundary;
    unsigned int sizeIndex = (lastOAM->word1 & 0xc000) >> 14;
    unsigned int shapeIndex = (lastOAM->word0 & 0xc000) >> 14;
    unsigned int logSizeBytes = 5;
    switch (shapeIndex)
    {
    case 0: // square
        switch (sizeIndex)
        {
        case 0: // 8x8
            logSizeBytes = 5; break;
        case 1: // 16x16
            logSizeBytes = 7; break;
        case 2: // 32x32
            logSizeBytes = 9; break;
        case 3: // 64x64
            logSizeBytes = 11; break;
        }
        break;
    case 1: // horizontal
        switch (sizeIndex)
        {
        case 0: // 16x8
            logSizeBytes = 6; break;
        case 1: // 32x8
            logSizeBytes = 7; break;
        case 2: // 32x16
            logSizeBytes = 8; break;
        case 3: // 64x32
            logSizeBytes = 10; break;
        }
        break;
    case 2: // vertical
        switch (sizeIndex)
        {
        case 0: // 8x16
            logSizeBytes = 6; break;
        case 1: // 8x32
            logSizeBytes = 7; break;
        case 2: // 16x32
            logSizeBytes = 8; break;
        case 3: // 32x64
            logSizeBytes = 10; break;
        }
        break;
    }
    if (logAlignmentBoundary < logSizeBytes)
        logAlignmentBoundary = logSizeBytes;
    usedObjVRAMEnd_48_ = objVRAMOffset + (1 << logAlignmentBoundary);
    return false;
}

bool OBJSpriteSet::LoadNANR(void* data, unsigned int length, SafeAllocator* alloc)
{
    Nitro2DAnimationBank* animBank;
    if (data == NULL)
        data = NULL;
    else if (length == NULL)
        data = NULL;
    else if (!Nitro2D_GetAnimationBank(data, &animBank))
        data = NULL;

    if (data == NULL)
        return true;

    Sprite* sprite;
    Nitro2DAnimationBank::AnimationBlock* block;
    List* animList = sprites_3c_;
    if (animList != NULL && alloc != NULL)
    {
        unsigned short i;
        unsigned short numSprites = animBank->numAnimBlocks;
        Sprite* spriteBuffer = (Sprite*)alloc->Allocate(numSprites * sizeof(Sprite));
        animList->sprites_ = spriteBuffer;
        animList->count_ = numSprites;

        for (i = 0; i < numSprites; i++)
        {
            block = Nitro2D_AnimationBankGetAnimationBlock(animBank, i);
            if (block == NULL)
                continue;
            sprite = &spriteBuffer[i];
            sprite->Reset();
            sprite->animationType_0 = block->animationType;
            sprite->numFrames_a = block->numFrames;
            sprite->id_8 = i;
            AnimationFrame* frameBuffer = 
                (AnimationFrame*)alloc->Allocate(sprite->numFrames_a * sizeof(AnimationFrame));
            sprite->frames_c = frameBuffer;
            for (unsigned short j = 0; j < sprite->numFrames_a; j++)
            {
                Nitro2DAnimationBank::FrameBlock* nitroFrame = &block->pFrames[j];
                if (nitroFrame == NULL)
                    continue;
                AnimationFrame* frame = &frameBuffer[j];
                frame->Reset();
                frame->duration_0 = nitroFrame->duration;
                frame->metatileIndex_2 = nitroFrame->metatileIndices[0];
            }
        }
    }
    return false;
}

bool OBJSpriteSet::LoadNCGR(void* data, unsigned int length)
{
    Nitro2D_ClearCharacterLoadDesc(&charLoad_0_);
    Nitro2DCharacter* character;
    if (GetCharBlockFromNitroFile(&character, data, length) != false)
        return true;
    
    int vramType = (engine_50_ == 0 ? 1 : 2);
    if (alignmentBoundaryType_38_ == 4)
        Nitro2D_LoadCharacterToVRAM2(character, 0, vramType, &charLoad_0_);
    else
        Nitro2D_LoadCharacterToVRAM(character, charVRAMLoadOffset_44_, vramType, &charLoad_0_);
    return false;
}

bool OBJSpriteSet::LoadNCLR(void* data, unsigned int length)
{
    Nitro2D_ClearPaletteLoadDesc(&paletteLoad_24_);
    Nitro2DPalette* palette = NULL;
    Nitro2DPaletteCMP* paletteCMP = NULL;
    CBool hasCMP = Nitro2D_GetPaletteCMP(data, &paletteCMP);

    bool error;
    if (data == NULL)
        error = true;
    else if (length == NULL)
        error = true;
    else if (Nitro2D_GetPalette(data, &palette))
        error = false;
    else
        error = true;

    if (error)
        return true;

    int vramType = (engine_50_ == 0 ? 1 : 2);
    if (hasCMP)
        Nitro2D_LoadPaletteWithCMPToVRAM(palette, paletteCMP, 0, vramType, &paletteLoad_24_);
    else
        Nitro2D_LoadPaletteToVRAM(palette, 0, vramType, &paletteLoad_24_);

    return false;
}

void OBJSpriteSet::DrawMetatile(Metatile* metatile) 
{
    if (metatile == NULL)
        return;

    void* objManager = func_0203bd08();
    OAMEntryEXT* oamTargetBuffer;
    Matrix2x2* pTransform = NULL;
    oamTargetBuffer = func_0203be4c(objManager);
    if (engine_50_ == 0)
        oamTargetBuffer = func_0203be40(objManager);
    bool doubleSize = false;
    Matrix2x2 rotScale;
    Matrix2x2 rotInvScale;
    if (metatile->useTransform_23)
    {
        Compute2x2Matrices(&rotScale, &rotInvScale, metatile);
        pTransform = &rotScale;
        if (metatile->doubleSize_24)
            doubleSize = true;
    }

    unsigned short firstObjIdx = metatile->destinationIndex_22 & 0x7f;
    unsigned short objCapacity = 128 - firstObjIdx;

    if (pTransform != NULL)
    {
        OAMEntryEXT* rotScaleTarget = &oamTargetBuffer[metatile->rotScaleDataIndex_20 * 4];
        rotScaleTarget[0].rotScaleData = rotInvScale.entries[0] >> 4;
        rotScaleTarget[1].rotScaleData = rotInvScale.entries[1] >> 4;
        rotScaleTarget[2].rotScaleData = rotInvScale.entries[2] >> 4;
        rotScaleTarget[3].rotScaleData = rotInvScale.entries[3] >> 4;
    }

    Vector2fix position = metatile->position_14;
    if (position.y < -64 << 12 || position.y >= 192 << 12)
        return;
    
    int paletteIdx, priority;
    OAMEntryEXT* oamTarget = &oamTargetBuffer[firstObjIdx];
    Nitro2D_DrawMetatile(oamTarget, objCapacity, &metatile->nitroData_0, 
        pTransform, &metatile->position_14, metatile->rotScaleDataIndex_20, doubleSize);
    paletteIdx = metatile->paletteIndex_25;
    priority = metatile->priority_26;
    if (paletteIdx < 0 && priority < 0)
        return;
    
    int objMode = GetMetatileObjMode(metatile);
    for (unsigned short i = metatile->nitroData_0.numObjs; i != 0; i--)
    {
        if (paletteIdx >= 0)
        {
            oamTarget->words01 = (oamTarget->words01 & ~0xc00) | (objMode << 10);
            oamTarget->base.word2 = (oamTarget->base.word2 & ~0xf000) | (paletteIdx << 12);
        }
        if (priority >= 0)
            oamTarget->base.word2 = (oamTarget->base.word2 & ~0xc00) | (priority << 10);
        oamTarget++;
    }
}

static inline bool TestSpriteBit3(OBJSpriteSet::Sprite* sprite)
{
    if (sprite != NULL)
        return sprite->flags_15 & 8;
    return false;
}

void OBJSpriteSet::DrawSprite(unsigned char id)
{
    if (sprites_3c_ == NULL)
        return;
    Sprite* sprite = sprites_3c_->GetSprite(id);
    if (sprite == NULL)
        return;
    if (!TestSpriteBit3(sprites_3c_->GetSprite(id)))
        return;
    unsigned short tileIndex = sprite->frames_c[sprite->currentFrame_10].metatileIndex_2;
    if (tileIndex >= numMetatiles_4c_)
        return;
    Metatile* tile = &metatiles_40_[tileIndex];
    short drawX = sprite->unknown_4;
    short drawY = sprite->unknown_6;
    tile->position_14.x = drawX << 12;
    tile->position_14.y = drawY << 12;
    if (sprite->destinationIndex_14 != 0xff)
        tile->destinationIndex_22 = sprite->destinationIndex_14;
    DrawMetatile(tile);
}

void OBJSpriteSet::DrawAll()
{
    if (sprites_3c_ == NULL)
        return;
    unsigned int numSprites = sprites_3c_->count_;
    for (unsigned short i = 0; i < numSprites; i++)
        DrawSprite(i);
}

void OBJSpriteSet::AssignOBJIndices(Metatile* metatiles, unsigned short numMetatiles, unsigned char firstIndex)
{
    if (metatiles == NULL)
        return;
    for (unsigned short i = 0; i < numMetatiles; i++)
    {
        Metatile* tile = &metatiles[i];
        tile->destinationIndex_22 += firstIndex;
        tile->destinationIndex_22 &= 0x7f;
    }
}

unsigned short OBJSpriteSet::GetMetatilePalette(const Metatile* metatile) const
{
    if (metatile == NULL)
        return 0;
    return GetMetatilePaletteInternal(metatile);
}

void OBJSpriteSet::SetMetatileTransformFlags(Metatile* metatile, bool transformEnabled, bool doubleSize)
{
    if (metatile == NULL)
        return;
    unsigned short flags = transformEnabled;
    flags |= (unsigned short)(doubleSize << 1);
    if (SetOAMWord(metatile, 0, 0x300, 8, flags))
        return;
    metatile->useTransform_23 = transformEnabled;
    metatile->doubleSize_24 = doubleSize;
}

void OBJSpriteSet::SetMetatilePriority(Metatile* metatile, unsigned char priority)
{
    SetOAMWord(metatile, 2, 0xc00, 10, priority & 3);
}

void OBJSpriteSet::SetMetatilePalette(Metatile* metatile, unsigned short palette)
{
    SetOAMWord(metatile, 2, 0xf000, 12, palette & 15);
}

void OBJSpriteSet::SetMetatileOBJMode(Metatile* metatile, unsigned char mode)
{
    SetOAMWord(metatile, 0, 0xc00, 10, mode & 3);
}

bool OBJSpriteSet::SetOAMWord(Metatile *metatile, unsigned int wordIdx, unsigned short clearMask,
    unsigned short setShift, unsigned short setMask)
{
    if (metatile == NULL)
        return true;
    if (wordIdx >= 3)
        return true;
    if (metatile == NULL) // again...
        return true;
    setMask <<= setShift;
    for (unsigned short i = 0; i < metatile->nitroData_0.numObjs; i++)
    {
        OAMEntry* loopEntry = &metatile->nitroData_0.pAttributes[i];
        unsigned short word;
        switch (wordIdx)
        {
        case 0:
            word = loopEntry->word0;
            break;
        case 1:
            word = loopEntry->word1;
            break;
        case 2:
            word = loopEntry->word2;
            break;
        }
        word &= ~clearMask;
        word |= setMask;
        switch (wordIdx)
        {
        case 0:
            loopEntry->word0 = word;
            break;
        case 1:
            loopEntry->word1 = word;
            break;
        case 2:
            loopEntry->word2 = word;
            break;
        }
    }
    return false;
}

void OBJSpriteSet::Compute2x2Matrices(Matrix2x2* outRotScale, Matrix2x2* outRotInvScale, const Metatile* metatile)
{
    if (metatile == NULL || outRotScale == NULL || outRotInvScale == NULL)
        return;
    
    Matrix2x2 rotMatrix;
    Mat2x2_WriteRotation(&rotMatrix, LookupSine(metatile->normalizedAngle_1c >> 4), LookupCosine(metatile->normalizedAngle_1c >> 4));
    Mat2x2_ApplyScale(&rotMatrix, outRotScale, metatile->unknown_c, metatile->unknown_10);
    fix32_t invX = fix32_Divide(1 << 12, metatile->unknown_c);
    fix32_t invY = fix32_Divide(1 << 12, metatile->unknown_10);
    Mat2x2_ApplyScale(&rotMatrix, outRotInvScale, invX, invY);
}

int OBJSpriteSet::GetMetatileObjMode(const Metatile* metatile) const
{
    if (metatile == NULL)
        return 0;
    unsigned int word0 = metatile->nitroData_0.pAttributes[0].word0;
    return (word0 >> 10) & 3;
}

unsigned int OBJSpriteSet::GetMetatilePaletteInternal(const Metatile* metatile) const
{
    if (metatile == NULL)
        return 0;
    unsigned int word = metatile->nitroData_0.pAttributes[0].word2;
    return (word >> 12) & 15;
}