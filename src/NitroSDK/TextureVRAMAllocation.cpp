#include "NitroSDK/VRAMAllocation.h"
#include <globaldefs.h>

// sizeof == 0x18.
// block in VRAM corresponding to a VRAM bank, or half of one, that's mapped for
// texture image data
struct TextureImageVRAMBlock
{
    unsigned int freeStart;
    unsigned int freeEnd;
    CBool available;
    CBool isHalfSize;
    unsigned short blockIndex;
    unsigned short pad_12; // filled with 0xffff, might be 2 chars
    // offset, within the overall VRAM assigned to texture image, of this block
    unsigned int blockOffset;

    inline unsigned int GetLength() const
    {
        return freeEnd - freeStart;
    }
} static s_blocks[5] =
{
    { -1, -1, false, false, 0, -1, 0 },
    { -1, -1, false, true,  1, -1, 0x20000 },
    { -1, -1, false, true,  2, -1, 0x30000 },
    { -1, -1, false, false, 3, -1, 0x40000 },
    { -1, -1, false, false, 4, -1, 0x60000 }
};

static unsigned short s_numImageBanks;

static TextureImageVRAMBlock* s_compressedBlockPriorityList[2] =
{
    &s_blocks[0],
    &s_blocks[3]
};

static TextureImageVRAMBlock* s_blockPriorityList[5] = 
{
    &s_blocks[4],
    &s_blocks[3],
    &s_blocks[0],
    &s_blocks[2],
    &s_blocks[1]
};

static void InitializeTextureImageVRAMBlocks();

static void SetTextureImageAllocationOrder(int first, int second, int third, int fourth, int fifth)
{
    s_blockPriorityList[0] = &s_blocks[first];
    s_blockPriorityList[1] = &s_blocks[second];
    s_blockPriorityList[2] = &s_blocks[third];
    s_blockPriorityList[3] = &s_blocks[fourth];
    s_blockPriorityList[4] = &s_blocks[fifth];
}

void InitializeTextureImageAllocation(unsigned int numBanks, CBool setAllocFunctions)
{
    if (numBanks <= 2)
        SetTextureImageAllocationOrder(4, 3, 2, 0, 1);
    else
        SetTextureImageAllocationOrder(4, 3, 0, 2, 1);

    s_numImageBanks = numBanks;
    InitializeTextureImageVRAMBlocks();

    if (setAllocFunctions)
    {
        g_pfnAllocTextureImage = &FrameAllocateTextureImageVRAM;
        g_pfnFreeTextureImage = &FrameFreeTextureImageVRAM;
    }
}

static void InitializeTextureImageVRAMBlocks()
{
    int i;
    TextureImageVRAMBlock* loopObj = s_blocks;
    int numBlocksAvailable = s_numImageBanks;
    if (s_numImageBanks > 1)
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

    if (size >= 0x7fff0) // basically 512k (all of banks A-D)
        return 0;

    CBool success;
    if (compressed)
    {
        // see gbatek texture format 5 ("Format 5: 4x4-Texel Compressed Texture")
        TextureImageVRAMBlock* paletteIndexStorageBlock;
        TextureImageVRAMBlock* texelStorageBlock;
        for (int i = 0; i < 2; i++)
        {
            texelStorageBlock = s_compressedBlockPriorityList[i];
            if (!texelStorageBlock->available || texelStorageBlock->GetLength() < size)
                continue;
            
            switch (texelStorageBlock->blockIndex)
            {
            case 0:
                paletteIndexStorageBlock = &s_blocks[1];
                break;
            case 3:
                paletteIndexStorageBlock = &s_blocks[2];
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
            TextureImageVRAMBlock* obj = s_blockPriorityList[i];
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
    TextureImageVRAMBlock* loopObject = s_blocks;
    for (i = 0; i < 5; loopObject++, i++)
    {
        *(output + 2 * i) = loopObject->freeStart;
        *(output + 2 * i + 1) = loopObject->freeEnd;
    }
}

void RestoreTextureImageAllocationState(const unsigned int* input)
{
    int i = 0;
    int twoi = 0;
    TextureImageVRAMBlock* loopObject = s_blocks;
    while (i < 5)
    {
        loopObject->freeStart = *(input + twoi);
        loopObject->freeEnd = *(input + twoi + 1);
        twoi += 2;
        i++;
        loopObject++;
    }
}
