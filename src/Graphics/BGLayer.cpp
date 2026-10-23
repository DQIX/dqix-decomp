#pragma ipa file

#include "Graphics/BGLayer.h"
#include "NitroSDK/HWRegisters.h"
#include "Graphics/VRAMStaging.h"
#include "NitroSDK/Cache.h"
#include <globaldefs.h>

void SetBG3SubPriority(int to);
void SetBG2SubPriority(int to);
void SetBG1SubPriority(int to);
void SetBG0SubPriority(int to);
void SetBG3Priority(int to);
void SetBG2Priority(int to);
void SetBG1Priority(int to);
void SetBG0Priority(int to);

void SetBG3SubOffset(int x, int y);
void SetBG2SubOffset(int x, int y);
void SetBG1SubOffset(int x, int y);
void SetBG0SubOffset(int x, int y);
void SetBG3Offset(int x, int y);
void SetBG2Offset(int x, int y);
void SetBG1Offset(int x, int y);
void SetBG0Offset(int x, int y);

static inline VRAMLoadProc GetPaletteVRAMLoadProc(int engine)
{
    static VRAMLoadProc loadProcLookupPalette[] = 
    {
        LoadToMainBGStandardPalette,
        LoadToSubBGStandardPalette
    };
    return loadProcLookupPalette[engine];
}

static inline VRAMSubregion GetCharVRAMSubregion(int engine, int bgIndex)
{
    static const int regionLookupBG0Char[] =
    {
        VRAMSubregion_MainBG0Character,
        VRAMSubregion_SubBG0Character
    };
    return (VRAMSubregion)(regionLookupBG0Char[engine] + 2 * bgIndex);
}

static inline VRAMSubregion GetPaletteVRAMRegion(int engine)
{
    static const VRAMSubregion regionLookupPalette[] =
    { 
        VRAMSubregion_MainBGStandardPalette,
        VRAMSubregion_SubBGStandardPalette
    };
    return regionLookupPalette[engine];
}

static inline VRAMSubregion GetScreenVRAMSubregion(int engine, int bgIndex)
{
    static const int regionLookupBG0Screen[] =
    {
        VRAMSubregion_MainBG0Screen,
        VRAMSubregion_SubBG0Screen
    };
    return (VRAMSubregion)(regionLookupBG0Screen[engine] + 2 * bgIndex);
}

unsigned int Unused_GetScreenDimensions(int bgMode, int sizeMode)
{
    static const unsigned int screenDimLookup[2][4] =
    {
        { 0x00200020, 0x00200040, 0x00400020, 0x00400040 },
        { 0x00100010, 0x00200020, 0x00400040, 0x00800080 }
    };
    return screenDimLookup[bgMode][sizeMode];
}

static inline unsigned int GetScreenDataSize(int bgMode, int sizeMode)
{
    static const unsigned int screenDataSizeLookup[2][4] =
    {
        { 0x800, 0x1000, 0x1000, 0x2000 },
        { 0x200, 0x800, 0x2000, 0x8000 }
    };
    return screenDataSizeLookup[bgMode][sizeMode];
}

typedef void (*SetBGPriorityProc)(int);
static inline SetBGPriorityProc GetPrioritySetterProc(int engine, int bgIndex)
{
    static SetBGPriorityProc setPriorityLookup[2][4] =
    {
        { SetBG0Priority, SetBG1Priority, SetBG2Priority, SetBG3Priority },
        { SetBG0SubPriority, SetBG1SubPriority, SetBG2SubPriority, SetBG3SubPriority }
    };
    return setPriorityLookup[engine][bgIndex];
}

typedef void (*SetBGOffsetProc)(int, int);
static inline SetBGOffsetProc GetOffsetSetterProc(int engine, int bgIndex)
{
    static SetBGOffsetProc setOffsetLookup[2][4] =
    {
        { SetBG0Offset, SetBG1Offset, SetBG2Offset, SetBG3Offset },
        { SetBG0SubOffset, SetBG1SubOffset, SetBG2SubOffset, SetBG3SubOffset },
    };
    return setOffsetLookup[engine][bgIndex];
}

VRAMLoadProc GetCharVRAMLoadProc(int engine, int bgIndex)
{
    static VRAMLoadProc loadBGCharLookup[2][4] =
    {
        { LoadToMainBG0CharacterData, LoadToMainBG1CharacterData, LoadToMainBG2CharacterData, LoadToMainBG3CharacterData },
        { LoadToSubBG0CharacterData, LoadToSubBG1CharacterData, LoadToSubBG2CharacterData, LoadToSubBG3CharacterData }
    };
    return loadBGCharLookup[engine][bgIndex];
}

VRAMLoadProc GetScreenVRAMLoadProc(int engine, int bgIndex)
{
    static VRAMLoadProc loadBGScreenLookup[2][4] = 
    {
        { LoadToMainBG0ScreenData, LoadToMainBG1ScreenData, LoadToMainBG2ScreenData, LoadToMainBG3ScreenData },
        { LoadToSubBG0ScreenData, LoadToSubBG1ScreenData, LoadToSubBG2ScreenData, LoadToSubBG3ScreenData }
    };
    return loadBGScreenLookup[engine][bgIndex];
}

void SetBG3SubPriority(int to) { BG3CNTSUB = (BG3CNTSUB & ~3) | to; }
void SetBG2SubPriority(int to) { BG2CNTSUB = (BG2CNTSUB & ~3) | to; }
void SetBG1SubPriority(int to) { BG1CNTSUB = (BG1CNTSUB & ~3) | to; }
void SetBG0SubPriority(int to) { BG0CNTSUB = (BG0CNTSUB & ~3) | to; }
void SetBG3Priority(int to) { BG3CNT = (BG3CNT & ~3) | to; }
void SetBG2Priority(int to) { BG2CNT = (BG2CNT & ~3) | to; }
void SetBG1Priority(int to) { BG1CNT = (BG1CNT & ~3) | to; }
void SetBG0Priority(int to) { BG0CNT = (BG0CNT & ~3) | to; }

void SetBG3SubOffset(int x, int y) { BG3OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG2SubOffset(int x, int y) { BG2OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG1SubOffset(int x, int y) { BG1OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG0SubOffset(int x, int y) { BG0OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG3Offset(int x, int y) { BG3OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG2Offset(int x, int y) { BG2OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG1Offset(int x, int y) { BG1OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG0Offset(int x, int y) { BG0OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }

void BGLayer::CharacterData::Reset()
{
    vramOffset = 0;
    numTileEntries = 0;
    unknown_6 = 0;
    unknown_8 = 0;
    use8bpp = false;
}

int BGLayer::CharacterData::LoadAndStageFromBNCG(VRAMLoadProc proc, const void *data, SafeAllocator*)
{
    const char* source = (const char*)data;
    unsigned int length;
    char signature[4];
    memcpy(signature, source, 4);
    memcpy(&numTileEntries, source + 4, 2);
    memcpy(&unknown_6, source + 6, 2);
    memcpy(&unknown_8, source + 8, 2);
    memcpy(&use8bpp, source + 0xa, 1);
    memcpy(&unknown_b, source + 0xb, 1);
    memcpy(&length, source + 0xc, 4);
    CleanInvalidateCacheRange(source + 0x10, length);
    proc(source + 0x10, vramOffset, length);
    return 0;
}

void BGLayer::CharacterData::LoadMetadataFromBNCG(const void *data)
{
    const char* source = (const char*)data;
    unsigned int length;
    char signature[4];
    memcpy(signature, source, 4);
    memcpy(&numTileEntries, source + 4, 2);
    memcpy(&unknown_6, source + 6, 2);
    memcpy(&unknown_8, source + 8, 2);
    memcpy(&use8bpp, source + 0xa, 1);
    memcpy(&unknown_b, source + 0xb, 1);
    memcpy(&length, source + 0xc, 4);
    CleanInvalidateCacheRange(source + 0x10, length);
}

void BGLayer::CharacterData::StageBNCG(VRAMLoadProc proc, const void* data)
{
    if (data == NULL)
        return;
    const char* source = (const char*)data;
    unsigned int length;
    memcpy(&length, source + 0xc, 4);
    proc(source + 0x10, vramOffset, length);
}

void BGLayer::CharacterData::StageBNCG(int engine, int bgIndex, const void* data)
{
    if (data == NULL)
        return;
    const char* source = (const char*)data;
    source += 0xc;
    unsigned int length;
    memcpy(&length, source, 4);
    source += 0x4;

    StageMemoryToVRAM(GetCharVRAMSubregion(engine, bgIndex),
        source, vramOffset, length, true, false);
}

unsigned int BGLayer::CharacterData::GetEndVRAMOffset() const
{
    int bitshift = 5;
    if (use8bpp)
        bitshift = 6;
    return vramOffset + (numTileEntries << bitshift);
}

unsigned int BGLayer::CharacterData::GetVRAMSize() const
{
    int bitshift = 5;
    if (use8bpp)
        bitshift = 6;
    return (numTileEntries << bitshift);
}

int BGLayer::PaletteData::LoadAndStageFromBNCL(VRAMLoadProc proc, const void* data, SafeAllocator*)
{
    const char* source = (const char*)data;
    unsigned int length;
    char signature[4];
    memcpy(signature, source, 4);
    memcpy(&useExtPalette, source + 4, 1);
    char unknownWastedCopy[4];
    memcpy(unknownWastedCopy, source + 5, 3);
    memcpy(&length, source + 8, 4);
    if (palettesInUse == 0xffff)
    {
        CleanInvalidateCacheRange(source + 12, length);
        proc(source + 12, 0, length);
    }
    else
    {
        unsigned char bitmask = palettesInUse;
        for (int i = 0; i < 16; i++)
        {
            if (bitmask & 1)
            {
                CleanInvalidateCacheRange(source + 12 + i * 32, 32);
                proc(source + 12 + i * 32, i * 32, 32);
            }
            bitmask >>= 1;
        }
    }
    return 0;
}

void BGLayer::PaletteData::LoadMetadataFromBNCL(const void* data)
{
    const char* source = (const char*)data;
    unsigned int length;
    memcpy(&length, source + 8, 4);
    if (palettesInUse == 0xffff)
    {
        CleanInvalidateCacheRange(source + 12, length);
    }
    else
    {
        unsigned char bitmask = palettesInUse;
        for (int i = 0; i < 16; i++)
        {
            if (bitmask & 1)
                CleanInvalidateCacheRange(source + 12 + i * 32, 32);
            bitmask >>= 1;
        }
    }
}


void BGLayer::PaletteData::StageBNCL(VRAMLoadProc proc, const void* data)
{
    const char* source = (const char*)data;
    uint32_t length;
    memcpy(&length, source + 8, 4);
    if (palettesInUse == 0xffff)
    {
        proc(source + 12, 0, length);
    }
    else
    {
        unsigned char bitmask = palettesInUse;
        for (int i = 0; i < 16; i++)
        {
            if (bitmask & 1)
                proc(source + 12 + i * 32, i * 32, 32);
            bitmask >>= 1;
        }
    }
}

void BGLayer::PaletteData::StageBNCL(int engine, const void* data)
{
    const char* source = (const char*)data + 4;
    source += 4;
    uint32_t length;
    memcpy(&length, source, 4);
    source += 4;

    StageMemoryToVRAM(GetPaletteVRAMRegion(engine), source, 0, length, true, false);
}

void BGLayer::Sprite::Reset()
{
    width = 0;
    height = 0;
    unknown_4 = 0;
    unknown_5 = 0;
    bufferSize = 0;
    indexBuffer = NULL;
}

void BGLayer::Sprite::MaybeDestroy()
{
    Reset();
}

int BGLayer::Sprite::LoadFromBNSC(VRAMLoadProc unusedProc, const void* data, SafeAllocator* alloc)
{
    const char* source = (const char*)data;
    char signature[4];
    memcpy(signature, source, 4);
    memcpy(&width, source + 4, 2);
    memcpy(&height, source + 6, 2);
    memcpy(&unknown_4, source + 8, 1);
    memcpy(&unknown_5, source + 9, 1);
    memcpy(&unknown_6, source + 0xa, 2);
    memcpy(&bufferSize, source + 0xc, 4);
    indexBuffer = (unsigned short*)alloc->Allocate(bufferSize);
    memcpy(indexBuffer, source + 0x10, bufferSize);
    return 0;
}

void BGLayer::Sprite::GetLengthAndClearCache(const void* data)
{
    const char* source = (const char*)data;
    source += 0xc; // writing this way to avoid getting inlined below
    memcpy(&bufferSize, source, 4);
    source += 0x4;
    CleanInvalidateCacheRange(source, bufferSize);
}

void BGLayer::Sprite::StageBNSC(VRAMLoadProc proc, const void* data)
{
    const char* source = (const char*)data + 0x8;
    source += 0x4; // writing this way to avoid getting inlined below
    memcpy(&bufferSize, source, 4);
    source += 0x4;
    proc(source, 0, bufferSize);
}

void BGLayer::Sprite::StageBNSC(int engine, int bgIndex, const void* data)
{
    const char* source = (const char*)data;
    memcpy(&bufferSize, source + 0xc, 4);

    StageMemoryToVRAM(GetScreenVRAMSubregion(engine, bgIndex),
        source + 0x10, 0, bufferSize, true, false);
}

void BGLayer::Sprite::SwitchPalette(unsigned short paletteIndex)
{
    if (indexBuffer == NULL)
        return;
    SwitchPaletteInRectangle(paletteIndex, 0, 0, width, height);
}

void BGLayer::Sprite::SwitchPaletteInRectangle(unsigned short paletteIdx,
    short left, short top, short w, short h)
{
    if (indexBuffer == NULL)
        return;
    unsigned short* indices = indexBuffer;
    
    unsigned short paletteMask = paletteIdx & 0xf;
    paletteMask <<= 12;
    short right = left + w;
    short bottom = top + h;
    short rowStride = width;

    for (short y = top; y < bottom; y++)
    {
        unsigned short* dst = indices + rowStride * y + left;
        for (short x = left; x < right; dst++, x++)
        {
            *dst &= 0xfff;
            *dst |= paletteMask;
        }
    }
}

BGLayer::Sprite* BGLayer::GetScreenObject(unsigned int idx)
{
    if (sprites_ == NULL)
        return NULL;
    if (numSprites_ <= idx)
        return NULL;
    else
        return &sprites_[idx];
}

void BGLayer::AllocateScreenObjects(unsigned int count, SafeAllocator* alloc)
{
    if (alloc == NULL)
        return;
    sprites_ = (Sprite*)alloc->Allocate(count * sizeof(Sprite));
    spriteCapacity_ = count;
}

void BGLayer::Reset()
{
    charData_.Reset();
    paletteData_.useExtPalette = false;
    paletteData_.palettesInUse = 0xffff;
    sprites_ = NULL;
    indexBuffer_ = 0;
    unknown_18_ = 0;
    engine_ = 0;
    bgIndex_ = 0;
    numSprites_ = 0;
    spriteCapacity_ = 0;
    unknown_1f_ = 0;
}

void BGLayer::MaybeDestroy()
{
    indexBuffer_ = 0;
    charData_.Reset();
    paletteData_.useExtPalette = false;
    paletteData_.palettesInUse = 0xffff;
    if (sprites_ == NULL)
        return;
    for (unsigned char i = 0; i < numSprites_; i++)
        sprites_[i].MaybeDestroy();
}

void BGLayer::PrepareIndexBuffer(void* buffer)
{
    if (buffer == NULL)
        buffer = indexBuffer_;
    if (buffer == NULL)
        return;
    memset(buffer, 0, GetScreenDataSize(unknown_1f_, unknown_18_));
}

void BGLayer::CleanIndexBufferCache(const void* buffer)
{
    if (buffer == NULL)
        buffer = indexBuffer_;
    if (buffer == NULL)
        return;
    CleanInvalidateCacheRange(buffer, GetScreenDataSize(unknown_1f_, unknown_18_));
}

void BGLayer::UploadIndexBuffer(const void* buffer)
{
    if (buffer == NULL)
        buffer = indexBuffer_;
    if (buffer == NULL)
        return;
    unsigned int copyLength = GetScreenDataSize(unknown_1f_, unknown_18_);
    VRAMLoadProc proc = GetScreenVRAMLoadProc(this->engine_, this->bgIndex_);
    proc(buffer, 0, copyLength);
}

void BGLayer::CleanCacheAndUploadIndexBuffer(const void* buffer)
{
    if (buffer == NULL)
        buffer = indexBuffer_;
    if (buffer == NULL)
        return;
    CleanIndexBufferCache(buffer);
    UploadIndexBuffer(buffer);
}

void BGLayer::ConfigureTextMode(int sizeIndex)
{
    unknown_18_ = sizeIndex;
    unknown_1f_ = 0;
}

void BGLayer::AllocateIndexBuffer(SafeAllocator* alloc)
{
    if (alloc == NULL)
        return;
    unsigned int bufferLength = GetScreenDataSize(unknown_1f_, unknown_18_);
    indexBuffer_ = (unsigned short*)alloc->Allocate(bufferLength);
    memset(indexBuffer_, 0, bufferLength);
}

int BGLayer::ProcessGraphicsFile(const void* fileData, SafeAllocator* alloc)
{
    if (fileData == NULL)
        return true;
    
    char signature[5] = {0};
    memcpy(signature, fileData, 5);

    if (memcmp(signature, "CHAR", 4) == 0)
        return charData_.LoadAndStageFromBNCG(GetCharVRAMLoadProc(engine_, bgIndex_), fileData, alloc);
    else if (memcmp(signature, "PALT", 4) == 0)
        return paletteData_.LoadAndStageFromBNCL(GetPaletteVRAMLoadProc(engine_), fileData, alloc);
    else if (memcmp(signature, "SCRN", 4) == 0 && sprites_ != NULL)
    {
        if (spriteCapacity_ <= numSprites_)
            return 1;
        if (sprites_[numSprites_].LoadFromBNSC(GetScreenVRAMLoadProc(engine_, bgIndex_), fileData, alloc))
            return 1;
        numSprites_++;
        return 0;
    }

    return 1;
}

void BGLayer::ProcessGraphicsFileMetadata(const void* fileData)
{
    if (fileData == NULL)
        return;
    char signature[5] = {0};
    memcpy(signature, fileData, 5);

    if (memcmp(signature, "CHAR", 4) == 0)
        charData_.LoadMetadataFromBNCG(fileData);
    else if (memcmp(signature, "PALT", 4) == 0)
        paletteData_.LoadMetadataFromBNCL(fileData);
    else if (memcmp(signature, "SCRN", 4) == 0)
    {
        Sprite tempScreenData;
        tempScreenData.Reset();
        // just clear the cache at the part of the file containing the index buffer
        tempScreenData.GetLengthAndClearCache(fileData);
    }
}

void BGLayer::StageGraphicsFileBuffer(const void *fileData)
{
    if (fileData == NULL)
        return;
    char signature[5] = {0};
    memcpy(signature, fileData, 5);

    if (memcmp(signature, "CHAR", 4) == 0)
        charData_.StageBNCG(GetCharVRAMLoadProc(engine_, bgIndex_), fileData);
    else if (memcmp(signature, "PALT", 4) == 0)
        paletteData_.StageBNCL(GetPaletteVRAMLoadProc(engine_), fileData);
    else if (memcmp(signature, "SCRN", 4) == 0)
    {
        Sprite tempScreenData;
        tempScreenData.Reset();
        tempScreenData.StageBNSC(GetScreenVRAMLoadProc(engine_, bgIndex_), fileData);
    }
}

void BGLayer::StageGraphicsFileBuffer2(const void *fileData)
{
    if (fileData == NULL)
        return;
    char signature[5] = {0};
    memcpy(signature, fileData, 5);

    if (memcmp(signature, "CHAR", 4) == 0)
        charData_.StageBNCG(engine_, bgIndex_, fileData);
    else if (memcmp(signature, "PALT", 4) == 0)
        paletteData_.StageBNCL(engine_, fileData);
    else if (memcmp(signature, "SCRN", 4) == 0)
    {
        Sprite tempScreenData;
        tempScreenData.Reset();
        tempScreenData.StageBNSC(engine_, bgIndex_, fileData);
    }
}

void BGLayer::SetPriority(int to)
{
    GetPrioritySetterProc(engine_, bgIndex_)(to & 3);
}

void BGLayer::SetOffset(int x, int y)
{
    GetOffsetSetterProc(engine_, bgIndex_)(x, y);
}

void BGLayer::BlitSpriteToBuffer(unsigned short *destIdxBuffer, const Sprite *obj,
    short sourceTileX, short sourceTileY, short destTileX, short destTileY,
    short width, short height, unsigned short palette)
{
    if (destIdxBuffer == NULL || obj == NULL)
        return;
    // quit early if destination is completely off-screen
    if (destTileX >= 32 || destTileY >= 25)
        return;
    if (destTileX + width <= 0 || destTileY + height <= 0)
        return;
    if (width <= 0 || height <= 0)
        return;

    short sourceW = obj->width;
    short sourceH = obj->height;

    // quit early if source is entirely out of range
    if (sourceW <= sourceTileX || sourceH <= sourceTileY || 
        sourceTileX + width <= 0 || sourceTileY + height <= 0)
        return;
    
    if (sourceTileX < 0)
    {
        // rhs is positive!
        destTileX += -sourceTileX;
        width -= -sourceTileX;
        sourceTileX = 0;
    }
    else if (sourceW < sourceTileX + width)
        width = sourceW - sourceTileX;

    if (sourceTileY < 0)
    {
        destTileY += -sourceTileY;
        height -= -sourceTileY;
        sourceTileY = 0;
    }
    else if (sourceH < sourceTileY + height)
        height = sourceH - sourceTileY;

    if (sourceW < width)
        width = sourceW;
    if (sourceH < height)
        height = sourceH;

    if (destTileX < 0)
    {
        sourceTileX += -destTileX;
        width -= -destTileX;
        destTileX = 0;
    }
    else if (destTileX + width > 32)
        width = 32 - destTileX;
    
    if (destTileY < 0)
    {
        sourceTileY += -destTileY;
        height -= -destTileY;
        destTileY = 0;
    }
    else if (destTileY + height > 25)
        height = 25 - destTileY;

    const unsigned short* readPtr = obj->indexBuffer;
    if (readPtr == NULL)
        return;
    readPtr += sourceW * sourceTileY + sourceTileX;

    unsigned short* writePtr = destIdxBuffer + destTileY * 32 + destTileX;

    if (palette != 0xffff)
    {
        for (unsigned short y = 0; y < height; y++)
        {
            for (unsigned short x = 0; x < width; x++)
            {
                writePtr[x] = readPtr[x] & 0xfff;
                writePtr[x] |= (unsigned short)(palette << 12);
            }
            readPtr += sourceW;
            writePtr += 32;
        }
    }
    else
    {
        for (unsigned short y = 0; y < height; y++)
        {
            for (unsigned short x = 0; x < width; x++)
            {
                writePtr[x] = readPtr[x];
            }
            readPtr += sourceW;
            writePtr += 32;
        }
    }
}

void BGLayer::BlitSprite(const Sprite* obj, short sourceTileX, short sourceTileY,
    short destTileX, short destTileY, short width, short height, unsigned short palette)
{
    if (indexBuffer_ == NULL)
        return;
       
    BlitSpriteToBuffer(indexBuffer_, obj, sourceTileX, sourceTileY, 
        destTileX, destTileY, width, height, palette);
}

void BGLayer::BlitSpriteByIndex(unsigned int objIdx, short sourceTileX, short sourceTileY,
    short destTileX, short destTileY, short width, short height, unsigned short palette)
{
    if (numSprites_ <= objIdx)
        return;

    if (sprites_ == NULL)
        return;
    
    const Sprite* obj = &sprites_[objIdx];

    if (indexBuffer_ == NULL)
        return;

    BlitSpriteToBuffer(indexBuffer_, obj, 
        sourceTileX , sourceTileY, destTileX, destTileY,
        width, height, palette);
}

void BGLayer::BlitSprite(const Sprite* obj,
    short destTileX, short destTileY, unsigned short palette)
{
    if (obj == NULL)
        return;
    short w = obj->width;
    short h = obj->height;
    if (indexBuffer_ == NULL)
        return;
    BlitSpriteToBuffer(indexBuffer_, obj, 0, 0, destTileX, destTileY, w, h, palette);
}

void BGLayer::BlitSpriteByIndex(unsigned int objIdx,
    short destTileX, short destTileY, unsigned short palette)
{
    if (numSprites_ <= objIdx)
        return;
    if (sprites_ == NULL)
        return;
    BlitSprite(&sprites_[objIdx], destTileX, destTileY, palette);
}

void BGLayer::SetPaletteInRegion(short left, short top, short width, short height, unsigned short palette)
{
    if (indexBuffer_ == NULL)
        return;
    if (left + width < 0 || top + height < 0)
        return;
    if (left >= 32 || top >= 25)
        return;
    if (palette > 15)
        return;
    
    if (left < 0)
    {
        width -= -left;
        left = 0;
    }
    else if (left + width >= 32)
        width = 32 - left;

    if (top < 0)
    {
        height -= -top;
        top = 0;
    }
    else if (top + height >= 25)
        height = 25 - top;

    
    unsigned short* writePtr = indexBuffer_ + top * 32 + left;
    palette <<= 12;

    for (unsigned short y = 0; y < height; y++)
    {
        for (unsigned short x = 0; x < width; x++)
        {
            writePtr[x] &= 0xfff;
            writePtr[x] |= palette;
        }
        writePtr += 32;
    }
}

void BGLayer::BlitSpriteRepeated(const Sprite* obj, short width, short height,
        short destTileX, short destTileY, short countX, short countY, unsigned short palette)
{
    if (width <= 0 || height <= 0)
        return;
    if (obj->indexBuffer == NULL)
        return;

    short sourceW = obj->width;
    short sourceH = obj->height;
    if (sourceW < width)
        width = sourceW;
    if (sourceH < height)
        height = sourceH;

    short thisDrawX, thisDrawY;
    thisDrawY = destTileY;
    for (unsigned short y = 0; y < countY; y++)
    {
        thisDrawX = destTileX;
        for (unsigned short x = 0; x < countX; x++)
        {
            if (indexBuffer_ != NULL)
                BlitSpriteToBuffer(indexBuffer_, obj, 0, 0, thisDrawX, thisDrawY, width, height, palette);
            thisDrawX += width;
        }
        thisDrawY += height;
    }
}

void BGLayer::BlitSpriteByIndexRepeated(unsigned int objIdx, short width, short height,
        short destTileX, short destTileY, short countX, short countY, unsigned short palette)
{
    if (numSprites_ <= objIdx || sprites_ == NULL)
        return;

    BlitSpriteRepeated(&sprites_[objIdx], width, height,
        destTileX, destTileY, countX, countY, palette);
}

void BGLayer::UploadPaletteData(int whichPalette, int indexInPalette, const void* data, unsigned int dataLength)
{
    VRAMLoadProc proc = GetPaletteVRAMLoadProc(engine_);
    int bitShift = 5;
    if (paletteData_.useExtPalette)
    {
        indexInPalette = 0xff;
        bitShift = 9;
    }
    int trueOffset = (whichPalette & 0xf) << bitShift;
    trueOffset += (indexInPalette & 0xf) << 1;
    dataLength &= ~1;
    CleanInvalidateCacheRange(data, dataLength);
    proc(data, trueOffset, dataLength);
}

void BGLayer::SetTileInRectangle(unsigned short writeValue,
    short left, short top, short width, short height, unsigned short palette)
{
    if (indexBuffer_ == NULL)
        return;
    if (left + width <= 0 || top + height <= 0)
        return;
    if (left >= 32 || top >= 25)
        return;
    
    if (left < 0)
    {
        width -= -left;
        left = 0;
    }
    else if (left + width >= 32)
        width = 32 - left;

    if (top < 0)
    {
        height -= -top;
        top = 0;
    }
    else if (top + height >= 25)
        height = 25 - top;

    
    unsigned short* writePtr = indexBuffer_ + top * 32 + left;
    if (palette != 0xffff)
        palette <<= 12;
    else
        palette = 0;
    writeValue &= 0xfff;
    writeValue |= palette;

    for (short y = 0; y < height; y++)
    {
        for (short x = 0; x < width; x++)
        {
            writePtr[x] = writeValue;
        }
        writePtr += 32;
    }
}