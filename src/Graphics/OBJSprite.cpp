#pragma ipa file

#include "Graphics/OBJSprite.h"
#include "Resource/PAC.h"

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

    void func_020b0318(void*);
    void func_020b033c(void*);
}

extern char data_020f0504[]; // "RECN"
extern char data_020f0509[]; // "RNAN"
extern char data_020f050e[]; // "RGCN"
extern char data_020f0513[]; // "RLCN"

void OBJSprite::Metatile::Reset()
{
    memset(&nitroData_0, 0, sizeof(Nitro2DCellBank::Metatile));
    oamEntries_8 = NULL;
    unknown_c = unknown_10 = 1 << 12;
    position_14.x = position_14.y = 0;
    unknown_1c = 0;
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

void OBJSprite::Reset()
{
    memset(&charLoad_0_, 0, sizeof(Nitro2DCharacterLoadDesc));
    memset(&paletteLoad_24_, 0, sizeof(Nitro2DPaletteLoadDesc));
    unknown_38_ = 0;
    metatiles_40_ = NULL;
    unknown_44_ = 0;
    unknown_48_ = 0;
    numMetatiles_4c_ = 0;
    unknown_4e_ = 0;
    animations_3c_ = NULL;
    unknown_50_ = true;
}

void OBJSprite::Destroy()
{
}

bool OBJSprite::LoadFromPAC(void* data, unsigned int length, SafeAllocator* alloc)
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

bool OBJSprite::LoadFromNitroFile(void* data, unsigned int length, SafeAllocator* alloc)
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

bool OBJSprite::LoadNCER(void* argData, unsigned int length, SafeAllocator* alloc)
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
        numMetatiles_4c_ = cellBank->numMetatiles_0;
        metatiles_40_ = (Metatile*)alloc->Allocate(numMetatiles_4c_ * sizeof(Metatile));
        for (unsigned short i = 0; i < numMetatiles_4c_; i++)
            metatiles_40_[i].Reset();
    }

    Metatile* pMetatiles = metatiles_40_;
    Metatile* localMetatile;
    unsigned short numMetatiles = numMetatiles_4c_;
    
    if (pMetatiles != NULL && numMetatiles != 0 && alloc != NULL)
    {
        unsigned short sourceNumMetatiles = cellBank->numMetatiles_0;
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
            unsigned short thisNumObjs = nitroMetatile->numObjs_0;
            localMetatile = &pMetatiles[i];
            localMetatile->unknown_1e = i;
            localMetatile->destinationIndex_22 = objCounter;
            localMetatile->nitroData_0 = *nitroMetatile;
            localMetatile->nitroData_0.pAttributes_4 = localMetatile->oamEntries_8 =
                (OAMEntry*)alloc->Allocate(thisNumObjs * sizeof(OAMEntry));
            for (unsigned short j = 0; j < nitroMetatile->numObjs_0; j++)
            {
                localMetatile->oamEntries_8[j] = nitroMetatile->pAttributes_4[j];
            }
            objCounter += nitroMetatile->numObjs_0;
        }
    }

    unknown_4e_ = cellBank->numMetatiles_0;
    unknown_38_ = cellBank->unknown_8;

    Nitro2DCellBank::Metatile* lastNitroMT = Nitro2D_GetCellBankMetatile(cellBank, cellBank->numMetatiles_0 - 1);
    if (lastNitroMT == NULL)
        return true;
    unsigned int shift = unknown_38_ + 5;
    OAMEntry* lastOAM = &lastNitroMT->pAttributes_4[lastNitroMT->numObjs_0 - 1];
    
    unsigned int shiftedTileNum = (lastOAM->word2 & 0x3ff) << shift;
    unsigned int sizeIndex = (lastOAM->word1 & 0xc000) >> 14;
    unsigned int shapeIndex = (lastOAM->word0 & 0xc000) >> 14;
    unsigned int minShift = 5;
    switch (shapeIndex)
    {
    case 0: // square
        switch (sizeIndex)
        {
        case 0: // 8x8
            minShift = 5; break;
        case 1: // 16x16
            minShift = 7; break;
        case 2: // 32x32
            minShift = 9; break;
        case 3: // 64x64
            minShift = 11; break;
        }
        break;
    case 1: // horizontal
        switch (sizeIndex)
        {
        case 0: // 16x8
            minShift = 6; break;
        case 1: // 32x8
            minShift = 7; break;
        case 2: // 32x16
            minShift = 8; break;
        case 3: // 64x32
            minShift = 10; break;
        }
        break;
    case 2: // vertical
        switch (sizeIndex)
        {
        case 0: // 8x16
            minShift = 6; break;
        case 1: // 8x32
            minShift = 7; break;
        case 2: // 16x32
            minShift = 8; break;
        case 3: // 32x64
            minShift = 10; break;
        }
        break;
    }
    if (shift < minShift)
        shift = minShift;
    unknown_48_ = shiftedTileNum + (1 << shift);
    return false;
}

bool OBJSprite::LoadNANR(void* data, unsigned int length, SafeAllocator* alloc)
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

    Animation* localAnim;
    Nitro2DAnimationBank::AnimationBlock* block;
    AnimationList* animList = animations_3c_;
    if (animList != NULL && alloc != NULL)
    {
        unsigned short i;
        unsigned short numAnims = animBank->numAnimBlocks_0;
        Animation* animBuffer = (Animation*)alloc->Allocate(numAnims * sizeof(Animation));
        animList->data = animBuffer;
        animList->count = numAnims;

        for (i = 0; i < numAnims; i++)
        {
            block = Nitro2D_AnimationBankGetAnimationBlock(animBank, i);
            if (block == NULL)
                continue;
            localAnim = &animBuffer[i];
            localAnim->Reset();
            localAnim->unknown_0 = block->unknown_8;
            localAnim->numFrames_a = block->numFrames_0;
            localAnim->bufferIndex_8 = i;
            AnimationFrame* frameBuffer = 
                (AnimationFrame*)alloc->Allocate(localAnim->numFrames_a * sizeof(AnimationFrame));
            localAnim->frames_c = frameBuffer;
            for (unsigned short j = 0; j < localAnim->numFrames_a; j++)
            {
                Nitro2DAnimationBank::FrameBlock* nitroFrame = &block->unknown_c[j];
                if (nitroFrame == NULL)
                    continue;
                AnimationFrame* frame = &frameBuffer[j];
                frame->Reset();
                frame->unknown_0 = nitroFrame->unknown_4;
                frame->unknown_2 = nitroFrame->unknown_0[0];
            }
        }
    }
    return false;
}

bool OBJSprite::LoadNCGR(void* data, unsigned int length)
{
    func_020b0318(&charLoad_0_);
    Nitro2DCharacter* character;
    if (GetCharBlockFromNitroFile(&character, data, length) != false)
        return true;
    
    int vramType = (unknown_50_ == 0 ? 1 : 2);
    if (unknown_38_ == 4)
        Nitro2D_LoadCharacterToVRAM2(character, 0, vramType, &charLoad_0_);
    else
        Nitro2D_LoadCharacterToVRAM(character, unknown_44_, vramType, &charLoad_0_);
    return false;
}

bool OBJSprite::LoadNCLR(void* data, unsigned int length)
{
    func_020b033c(&paletteLoad_24_);
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

    int vramType = (unknown_50_ == 0 ? 1 : 2);
    if (hasCMP)
        Nitro2D_LoadPaletteWithCMPToVRAM(palette, paletteCMP, 0, vramType, &paletteLoad_24_);
    else
        Nitro2D_LoadPaletteToVRAM(palette, 0, vramType, &paletteLoad_24_);

    return false;
}

void OBJSprite::DrawMetatile(Metatile* metatile) 
{
    if (metatile == NULL)
        return;

    void* objManager = func_0203bd08();
    OAMEntryEXT* oamTargetBuffer;
    Matrix2x2* pTransform = NULL;
    oamTargetBuffer = func_0203be4c(objManager);
    if (unknown_50_ == 0)
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
    for (unsigned short i = metatile->nitroData_0.numObjs_0; i != 0; i--)
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

void OBJSprite::Compute2x2Matrices(Matrix2x2* outRotScale, Matrix2x2* outRotInvScale, const Metatile* metatile)
{
    if (metatile == NULL || outRotScale == NULL || outRotInvScale == NULL)
        return;
    
    Matrix2x2 rotMatrix;
    Mat2x2_WriteRotation(&rotMatrix, LookupSine(metatile->unknown_1c >> 4), LookupCosine(metatile->unknown_1c >> 4));
    Mat2x2_ApplyScale(&rotMatrix, outRotScale, metatile->unknown_c, metatile->unknown_10);
    fix32_t invX = fix32_Divide(1 << 12, metatile->unknown_c);
    fix32_t invY = fix32_Divide(1 << 12, metatile->unknown_10);
    Mat2x2_ApplyScale(&rotMatrix, outRotInvScale, invX, invY);
}

int OBJSprite::GetMetatileObjMode(const Metatile* metatile) const
{
    if (metatile == NULL)
        return 0;
    unsigned int word0 = metatile->nitroData_0.pAttributes_4[0].word0;
    return (word0 >> 10) & 3;
}