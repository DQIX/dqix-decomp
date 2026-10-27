#include "NitroSDK/VRAMAllocation.h"
#include <globaldefs.h>

// sizeof == 0x18.
// block in VRAM corresponding to a VRAM bank, or half of one, that's mapped for
// texture image data.
// We have an array of 5 of these (1 & 2 are half of the second mapped block,
// 0,3,4 are the full 1st, 3rd, 4th block respectively)
struct TextureImageVRAMBlock
{
    unsigned int freeStart;
    unsigned int freeEnd;
    CBool available;
    CBool isHalfSize;
    unsigned short blockIndex;
    unsigned short pad_12; // filled with 0xffff, might be 2 chars
    // offset, within the overall VRAM assigned to texture image, of this
    // block. e.g. the block corresponding to VRAM bank A is always 0 if mapped.
    // the five instances of this have values 0, 0x20000, 0x30000, 0x40000
    // and 0x60000 which matches the 4 banks A-D assigned to this with the second
    // being split into 2x 64K
    unsigned int blockOffset;

    inline unsigned int GetLength() const
    {
        return freeEnd - freeStart;
    }
} extern data_020f1f14[5];

extern unsigned short data_0210cf84; // #banks available for texture image data
struct PaletteAllocationData
{
    unsigned int freeStart;
    unsigned int freeEnd;
    unsigned int totalLength;
} extern data_0210cf88;

// to do: declare these here
extern TexturePaletteVRAMAllocProc data_020f1ef0;
extern TexturePaletteVRAMFreeProc data_020f1ef4;
extern TextureImageVRAMAllocProc data_020f1ee8;
extern TextureImageVRAMFreeProc data_020f1eec;

// points to data_020f1f14[...], indices { 0, 3, 4, 3, 0, 2, 1 }
extern TextureImageVRAMBlock* data_020f1ef8[7];

void InitializeTextureImageVRAMBlocks();
unsigned int FrameAllocateTextureImageVRAM(unsigned int requestedSize, CBool compressed, int unused);
int FrameFreeTextureImageVRAM(unsigned int);

unsigned int FrameAllocateTexturePaletteVRAM(unsigned int requestedSize, CBool eightByteAlign, int allocDir);
int FrameFreeTexturePaletteVRAM(unsigned int);

void ResetTexturePaletteAllocationState();

unsigned int InvalidTextureVRAMAlloc(unsigned int, CBool, int)
{
    return 0;
}

int InvalidTextureVRAMFree(unsigned int)
{
    return -1;
}

unsigned int InvalidPaletteVRAMAlloc(unsigned int, CBool, int)
{
    return 0;
}

int InvalidPaletteVRAMFree(unsigned int)
{
    return -1;
}

inline TextureImageVRAMBlock* GetVRAMBlockForUncompressedImageData(int i)
{
    TextureImageVRAMBlock* const* x = data_020f1ef8 + 2;
    return x[i];
}

void SetTextureImageAllocationOrder(int first, int second, int third, int fourth, int fifth)
{
    data_020f1ef8[2] = &data_020f1f14[first];
    data_020f1ef8[3] = &data_020f1f14[second];
    data_020f1ef8[4] = &data_020f1f14[third];
    data_020f1ef8[5] = &data_020f1f14[fourth];
    data_020f1ef8[6] = &data_020f1f14[fifth];
}

void InitializeTextureImageAllocation(unsigned int numBanks, CBool setAllocFunctions)
{
    if (numBanks <= 2)
        SetTextureImageAllocationOrder(4, 3, 2, 0, 1);
    else
        SetTextureImageAllocationOrder(4, 3, 0, 2, 1);

    data_0210cf84 = numBanks;
    InitializeTextureImageVRAMBlocks();

    if (setAllocFunctions)
    {
        data_020f1ee8 = &FrameAllocateTextureImageVRAM;
        data_020f1eec = &FrameFreeTextureImageVRAM;
    }
}

void InitializeTextureImageVRAMBlocks()
{
    int i;
    TextureImageVRAMBlock* loopObj = data_020f1f14;
    int numBlocksAvailable = data_0210cf84;
    if (data_0210cf84 > 1)
        numBlocksAvailable++;
    for (i = 0; i < 5; i++, loopObj++)
    {
        if (i < numBlocksAvailable)
            loopObj->available = true;
        else
            loopObj->available = false;

        if (loopObj->isHalfSize)
        {
            loopObj->freeStart = 0;
            loopObj->freeEnd = 0x10000; // 64k
        }
        else
        {
            loopObj->freeStart = 0;
            loopObj->freeEnd = 0x20000; // 128k
        }
    }
}

unsigned int FrameAllocateTextureImageVRAM(unsigned int requestedSize, CBool compressed, int unused)
{
    unsigned int chosenOffset;
    unsigned int size;
    
    if (requestedSize == 0)
        size = 16;
    else
        size = (requestedSize + 15) & ~15;

    if (size >= 0x7fff0) // 512kb, VRAM banks A-D?
        return 0;

    CBool success;
    if (compressed)
    {
        // see gbatek texture format 5 ("Format 5: 4x4-Texel Compressed Texture")
        TextureImageVRAMBlock* paletteIndexStorageBlock;
        TextureImageVRAMBlock* texelStorageBlock;
        for (int i = 0; i < 2; i++)
        {
            texelStorageBlock = data_020f1ef8[i];
            if (!texelStorageBlock->available || texelStorageBlock->GetLength() < size)
                continue;
            
            switch (texelStorageBlock->blockIndex)
            {
            case 0:
                paletteIndexStorageBlock = &data_020f1f14[1];
                break;
            case 3:
                paletteIndexStorageBlock = &data_020f1f14[2];
                break;
            default:
                paletteIndexStorageBlock = NULL;
                break;
            }

            if (!paletteIndexStorageBlock->available ||
                paletteIndexStorageBlock->GetLength() < (size / 2))
                continue;

            unsigned int oldStart = texelStorageBlock->freeStart;
            texelStorageBlock->freeStart += size;
            paletteIndexStorageBlock->freeStart += size / 2;
            chosenOffset = oldStart + texelStorageBlock->blockOffset;
            success = true;
            goto complete;
        }
        success = false;
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            TextureImageVRAMBlock* obj = GetVRAMBlockForUncompressedImageData(i);
            if (!obj->available || obj->GetLength() < size)
                continue;
            obj->freeEnd -= size;
            chosenOffset = obj->freeEnd + obj->blockOffset;
            success = true;
            goto complete;
        }
        success = false;
    }

complete:
    if (!success)
        return 0;
    return ((size >> 4) << 16) | (unsigned short)(chosenOffset >> 3) | (compressed << 31);
}

int FrameFreeTextureImageVRAM(unsigned int)
{
    return 0;
}

void SaveTextureImageAllocationState(unsigned int* output)
{
    int i;
    TextureImageVRAMBlock* loopObject = data_020f1f14;
    for (i = 0; i < 5; loopObject++, i++)
    {
        *(output + 2*i) = loopObject->freeStart;
        *(output + 2*i + 1) = loopObject->freeEnd;
    }
}

void RestoreTextureImageAllocationState(const unsigned int* input)
{
    int i = 0;
    int twoi = 0;
    TextureImageVRAMBlock* loopObject = data_020f1f14;
    for (; i < 5; twoi += 2, i++, loopObject++)
    {
        loopObject->freeStart = *(input + twoi);
        loopObject->freeEnd = *(input + twoi + 1);
    }
}

void InitializeTexturePaletteVRAMAllocation(unsigned int size, CBool setAllocFunctions)
{
    data_0210cf88.totalLength = size;
    ResetTexturePaletteAllocationState();
    if (setAllocFunctions)
    {
        data_020f1ef0 = &FrameAllocateTexturePaletteVRAM;
        data_020f1ef4 = &FrameFreeTexturePaletteVRAM;
    }
}

inline CBool FrameAllocateTexturePalette_Front(unsigned int size, CBool eightByte, unsigned int* pOutcome)
{
    unsigned int freeStart = data_0210cf88.freeStart;
    unsigned int distanceToAligned;
    if (eightByte)
        distanceToAligned = (8 - (freeStart & 7)) & 7;
    else
        distanceToAligned = (16 - (freeStart & 15)) & 15;
    unsigned int neededSize = size + distanceToAligned;

    if (data_0210cf88.freeEnd - data_0210cf88.freeStart >= neededSize)
    {
        unsigned int allocEnd = freeStart + neededSize;
        if (eightByte && allocEnd > 0x10000)
            return false;

        *pOutcome = freeStart + distanceToAligned;
        data_0210cf88.freeStart += neededSize;
        return true;
    }
    else
        return false;
}

inline CBool FrameAllocateTexturePalette_Back(unsigned int size, CBool eightByte, unsigned int* pOutcome)
{
    if (data_0210cf88.freeEnd >= size)
    {
        unsigned int freeStart = data_0210cf88.freeEnd - size;
        unsigned int alignmentWaste;
        if (eightByte)
            alignmentWaste = freeStart & 7;
        else
            alignmentWaste = freeStart & 15;
        unsigned int neededSize = size + alignmentWaste;

        if (data_0210cf88.freeEnd - data_0210cf88.freeStart >= neededSize)
        {
            unsigned int freeEnd = data_0210cf88.freeEnd;
            if (eightByte && freeEnd > 0x10000)
                return false;
            
            data_0210cf88.freeEnd -= neededSize;
            *pOutcome = data_0210cf88.freeEnd;
            return true;
        }
    }
    return false;
}

unsigned int FrameAllocateTexturePaletteVRAM(unsigned int size, CBool eightByte, int allocDir)
{
    unsigned int chosenOffset = 0;
    CBool success = false;

    size = (size == 0) ? 8 : ((size + 7) & ~7);

    if (size >= 0x7fff8) // 512kb
        return 0;

    if (allocDir == 1) // allocate at front
        success = FrameAllocateTexturePalette_Front(size, eightByte, &chosenOffset);
    else
        success = FrameAllocateTexturePalette_Back(size, eightByte, &chosenOffset);

    if (success)
        return ((size >> 3) << 16) | ((chosenOffset >> 3) & 0xffff);
    return 0;
}

int FrameFreeTexturePaletteVRAM(unsigned int)
{
    return 0;
}

void SaveTexturePaletteAllocationState(unsigned int* pRange)
{
    pRange[0] = data_0210cf88.freeStart;
    pRange[1] = data_0210cf88.freeEnd;
}

void RestoreTexturePaletteAllocationState(const unsigned int* pRange)
{
    data_0210cf88.freeStart = pRange[0];
    data_0210cf88.freeEnd = pRange[1];
}

void ResetTexturePaletteAllocationState()
{
    data_0210cf88.freeStart = 0;
    data_0210cf88.freeEnd = data_0210cf88.totalLength;
}