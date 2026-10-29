#include "NitroSDK/VRAMAllocation.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_020f1ef8 data_020f2064
#define data_020f1f14 data_020f2080

#define data_0210cf84 data_0210cc3c
#endif

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
} static data_020f1f14[5] =
{
    { -1, -1, false, false, 0, -1, 0 },
    { -1, -1, false, true,  1, -1, 0x20000 },
    { -1, -1, false, true,  2, -1, 0x30000 },
    { -1, -1, false, false, 3, -1, 0x40000 },
    { -1, -1, false, false, 4, -1, 0x60000 }
};

static unsigned short data_0210cf84; // #banks available for texture image data

// points to data_020f1f14[...], indices { 0, 3, 4, 3, 0, 2, 1 }
static TextureImageVRAMBlock* data_020f1ef8[7] = 
{
    &data_020f1f14[0],
    &data_020f1f14[3],
    &data_020f1f14[4],
    &data_020f1f14[3],
    &data_020f1f14[0],
    &data_020f1f14[2],
    &data_020f1f14[1]
};

void InitializeTextureImageVRAMBlocks();
unsigned int FrameAllocateTextureImageVRAM(unsigned int requestedSize, CBool compressed, int unused);
int FrameFreeTextureImageVRAM(unsigned int);

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
        g_pfnAllocTextureImage = &FrameAllocateTextureImageVRAM;
        g_pfnFreeTextureImage = &FrameFreeTextureImageVRAM;
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
