#pragma ipa file

#include "Graphics/OBJSpriteSet.h"
#include "Resource/PAC.h"
#include <asmhacks.h>

#if defined(jpn)
#define func_0203bd08 func_0203b760
#define func_0203be40 func_0203b898
#define func_0203be4c func_0203b8a4
#endif

static inline fix16_t LookupSine(int idx)
{
    return g_fixSinCosTable[2 * idx];
}

static inline fix16_t LookupCosine(int idx)
{
    return g_fixSinCosTable[2 * idx + 1];
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

void OBJSpriteSet::Metatile::Reset()
{
    memset(&nitroData, 0, sizeof(Nitro2DCellBank::Metatile));
    oamEntries = NULL;
    scale.x = scale.y = 1 << 12;
    position.x = position.y = 0;
    normalizedAngle = 0;
    id = 0;
    rotScaleDataIndex = 0;
    destinationObjIndex = 0;
    useTransform = false;
    doubleSize = false;
    paletteIndex = -1;
    priority = -1;
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
    if (!(sprite->flags & 2) || sprite->animationType == 0)
        return;
    sprite->frameTime += tickCount16;
    AnimationFrame* frame = &sprite->frames[sprite->currentFrame];
    if (frame->duration <= sprite->frameTime)
    {
        // don't simplify this to -=
        sprite->frameTime = sprite->frameTime - frame->duration;
        switch (sprite->animationType)
        {
        case 0:
            break;
        case 1: // stop at end
        {
            sprite->currentFrame++;
            if (sprite->numFrames <= sprite->currentFrame)
            {
                sprite->currentFrame = sprite->currentFrame - 1;
                sprite->frameTime = frame->duration;
                StopSpriteAnimation(sprite->id);
            }
            break;
        }
        case 2: // looping
            sprite->currentFrame++;
            if (sprite->numFrames <= sprite->currentFrame)
                sprite->currentFrame = 0;
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
    if (sprite->flags & 1) // if stopped, restart from beginning
    {
        sprite->currentFrame = 0;
        sprite->frameTime = 0;
    }
    sprite->flags &= ~7;
    sprite->flags |= 2;
}

void OBJSpriteSet::List::MaybePauseSpriteAnimation(unsigned int id)
{
    Sprite* sprite = GetSprite(id);
    if (sprite == NULL)
        return;
    sprite->flags &= ~7;
    sprite->flags |= 4;
}

void OBJSpriteSet::List::StopSpriteAnimation(unsigned int id)
{
    Sprite* sprite = GetSprite(id);
    if (sprite == NULL)
        return;
    sprite->flags &= ~7;
    sprite->flags |= 1;
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
        if (sprites_[i].id == id)
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
        sprite->destinationObjIndex = to;
}

void OBJSpriteSet::Reset()
{
    memset(&charLoad_, 0, sizeof(Nitro2DCharacterLoadDesc));
    memset(&paletteLoad_, 0, sizeof(Nitro2DPaletteLoadDesc));
    alignmentBoundaryType_ = 0;
    metatiles_ = NULL;
    charVRAMLoadOffset_ = 0;
    usedObjVRAMEnd_ = 0;
    numMetatiles_ = 0;
    numMetatilesCopy_ = 0;
    sprites_ = NULL;
    engine_ = true;
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
        return false;
    }
    return true;
}

bool OBJSpriteSet::LoadFromNitroFile(void* data, unsigned int length, SafeAllocator* alloc)
{
    char signatureString[5] = {0};
    memcpy(signatureString, data, 4);
    if (memcmp(signatureString, "RECN", 4) == 0)
    {
        if (LoadNCER(data, length, alloc))
            return true;
    }
    if (memcmp(signatureString, "RNAN", 4) == 0)
    {
        if (LoadNANR(data, length, alloc))
            return true;
    }
    else if (memcmp(signatureString, "RGCN", 4) == 0)
    {
        if (LoadNCGR(data, length))
            return true;
    }
    else if (memcmp(signatureString, "RLCN", 4) == 0)
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
        data = NULL; // ...
    else if (length == 0)
        data = NULL;
    else
    {
        // if you get here then data == argData, ...just why
        memcpy(data, argData, length);
        if (!Nitro2D_GetCellBank(data, &cellBank))
            data = NULL;
    }

    if (data == NULL)
        return true;

    if (alloc != NULL && metatiles_ == NULL)
    {
        numMetatiles_ = cellBank->numMetatiles;
        metatiles_ = (Metatile*)alloc->Allocate(numMetatiles_ * sizeof(Metatile));
        for (unsigned short i = 0; i < numMetatiles_; i++)
            metatiles_[i].Reset();
    }

    Metatile* pMetatiles = metatiles_;
    Metatile* localMetatile;
    unsigned short numMetatiles = numMetatiles_;
    
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
            localMetatile->id = i;
            localMetatile->destinationObjIndex = objCounter;
            localMetatile->nitroData = *nitroMetatile;
            localMetatile->nitroData.pAttributes = localMetatile->oamEntries =
                (OAMEntry*)alloc->Allocate(thisNumObjs * sizeof(OAMEntry));
            for (unsigned short j = 0; j < nitroMetatile->numObjs; j++)
            {
                localMetatile->oamEntries[j] = nitroMetatile->pAttributes[j];
            }
            objCounter += nitroMetatile->numObjs;
        }
    }

    numMetatilesCopy_ = cellBank->numMetatiles;
    alignmentBoundaryType_ = cellBank->alignmentBoundaryType;

    Nitro2DCellBank::Metatile* lastNitroMT = Nitro2D_GetCellBankMetatile(cellBank, cellBank->numMetatiles - 1);
    if (lastNitroMT == NULL)
        return true;
    unsigned int logAlignmentBoundary = alignmentBoundaryType_ + 5;
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
    usedObjVRAMEnd_ = objVRAMOffset + (1 << logAlignmentBoundary);
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
    List* spriteList = sprites_;
    if (spriteList != NULL && alloc != NULL)
    {
        Nitro2DAnimationBank::AnimationBlock* block;
        unsigned short i;
        unsigned short numSprites = animBank->numAnimBlocks;
        Sprite* spriteBuffer = (Sprite*)alloc->Allocate(numSprites * sizeof(Sprite));
        spriteList->sprites_ = spriteBuffer;
        spriteList->count_ = numSprites;

        for (i = 0; i < numSprites; i++)
        {
            block = Nitro2D_AnimationBankGetAnimationBlock(animBank, i);
            if (block == NULL)
                continue;
            sprite = &spriteBuffer[i];
            sprite->Reset();
            sprite->animationType = block->animationType;
            sprite->numFrames = block->numFrames;
            sprite->id = i;
            AnimationFrame* frameBuffer = 
                (AnimationFrame*)alloc->Allocate(sprite->numFrames * sizeof(AnimationFrame));
            sprite->frames = frameBuffer;
            for (unsigned short j = 0; j < sprite->numFrames; j++)
            {
                Nitro2DAnimationBank::FrameBlock* nitroFrame = &block->pFrames[j];
                if (nitroFrame == NULL)
                    continue;
                AnimationFrame* frame = &frameBuffer[j];
                frame->Reset();
                frame->duration = nitroFrame->duration;
                frame->metatileIndex = nitroFrame->metatileIndices[0];
            }
        }
    }
    return false;
}

bool OBJSpriteSet::LoadNCGR(void* data, unsigned int length)
{
    Nitro2D_ClearCharacterLoadDesc(&charLoad_);
    Nitro2DCharacter* character;
    if (GetCharBlockFromNitroFile(&character, data, length) != false)
        return true;
    
    int vramType = (engine_ == 0 ? 1 : 2);
    if (alignmentBoundaryType_ == 4)
        Nitro2D_LoadCharacterToVRAM2(character, 0, vramType, &charLoad_);
    else
        Nitro2D_LoadCharacterToVRAM(character, charVRAMLoadOffset_, vramType, &charLoad_);
    return false;
}

bool OBJSpriteSet::LoadNCLR(void* data, unsigned int length)
{
    Nitro2D_ClearPaletteLoadDesc(&paletteLoad_);
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

    int vramType = (engine_ == 0 ? 1 : 2);
    if (hasCMP)
        Nitro2D_LoadPaletteWithCMPToVRAM(palette, paletteCMP, 0, vramType, &paletteLoad_);
    else
        Nitro2D_LoadPaletteToVRAM(palette, 0, vramType, &paletteLoad_);

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
    if (engine_ == 0)
        oamTargetBuffer = func_0203be40(objManager);
    bool doubleSize = false;
    Matrix2x2 rotScale;
    Matrix2x2 rotInvScale;
    if (metatile->useTransform)
    {
        Compute2x2Matrices(&rotScale, &rotInvScale, metatile);
        pTransform = &rotScale;
        if (metatile->doubleSize)
            doubleSize = true;
    }

    unsigned short firstObjIdx = metatile->destinationObjIndex & 0x7f;
    unsigned short objCapacity = 128 - firstObjIdx;

    if (pTransform != NULL)
    {
        OAMEntryEXT* rotScaleTarget = &oamTargetBuffer[metatile->rotScaleDataIndex * 4];
        rotScaleTarget[0].rotScaleData = rotInvScale.entries[0] >> 4;
        rotScaleTarget[1].rotScaleData = rotInvScale.entries[1] >> 4;
        rotScaleTarget[2].rotScaleData = rotInvScale.entries[2] >> 4;
        rotScaleTarget[3].rotScaleData = rotInvScale.entries[3] >> 4;
    }

    Vector2fix position = metatile->position;
    if (position.y < -64 << 12 || position.y >= 192 << 12)
        return;
    
    int paletteIdx, priority;
    OAMEntryEXT* oamTarget = &oamTargetBuffer[firstObjIdx];
    Nitro2D_DrawMetatile(oamTarget, objCapacity, &metatile->nitroData, 
        pTransform, &metatile->position, metatile->rotScaleDataIndex, doubleSize);
    paletteIdx = metatile->paletteIndex;
    priority = metatile->priority;
    if (paletteIdx < 0 && priority < 0)
        return;
    
    int objMode = GetMetatileObjMode(metatile);
    for (unsigned short i = metatile->nitroData.numObjs; i != 0; i--)
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

// proably IsVisible()
static inline bool TestSpriteBit3(OBJSpriteSet::Sprite* sprite)
{
    if (sprite != NULL)
        return sprite->flags & 8;
    return false;
}

void OBJSpriteSet::DrawSprite(unsigned char id)
{
    if (sprites_ == NULL)
        return;
    Sprite* sprite = sprites_->GetSprite(id);
    if (sprite == NULL)
        return;
    if (!TestSpriteBit3(sprites_->GetSprite(id)))
        return;
    unsigned short tileIndex = sprite->frames[sprite->currentFrame].metatileIndex;
    if (tileIndex >= numMetatiles_)
        return;
    Metatile* tile = &metatiles_[tileIndex];
    short drawX = sprite->drawX;
    short drawY = sprite->drawY;
    tile->position.x = drawX << 12;
    tile->position.y = drawY << 12;
    if (sprite->destinationObjIndex != 0xff)
        tile->destinationObjIndex = sprite->destinationObjIndex;
    DrawMetatile(tile);
}

void OBJSpriteSet::DrawAll()
{
    if (sprites_ == NULL)
        return;
    unsigned int numSprites = sprites_->count_;
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
        tile->destinationObjIndex += firstIndex;
        tile->destinationObjIndex &= 0x7f;
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
    if (SetOAMWord(metatile, 0, 0x300, 8, flags) != false)
        return;
    metatile->useTransform = transformEnabled;
    metatile->doubleSize = doubleSize;
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
    for (unsigned short i = 0; i < metatile->nitroData.numObjs; i++)
    {
        OAMEntry* loopEntry = &metatile->nitroData.pAttributes[i];
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
    Mat2x2_WriteRotation(&rotMatrix, LookupSine(metatile->normalizedAngle >> 4), LookupCosine(metatile->normalizedAngle >> 4));
    Mat2x2_ApplyScale(&rotMatrix, outRotScale, metatile->scale.x, metatile->scale.y);
    fix32_t invX = fix32_Divide(1 << 12, metatile->scale.x);
    fix32_t invY = fix32_Divide(1 << 12, metatile->scale.y);
    Mat2x2_ApplyScale(&rotMatrix, outRotInvScale, invX, invY);
}

int OBJSpriteSet::GetMetatileObjMode(const Metatile* metatile) const
{
    if (metatile == NULL)
        return 0;
    unsigned int word0 = metatile->nitroData.pAttributes[0].word0;
    return (word0 >> 10) & 3;
}

unsigned int OBJSpriteSet::GetMetatilePaletteInternal(const Metatile* metatile) const
{
    if (metatile == NULL)
        return 0;
    unsigned int word = metatile->nitroData.pAttributes[0].word2;
    return (word >> 12) & 15;
}