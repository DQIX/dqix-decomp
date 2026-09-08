#include "Graphics/VRAMAllocations.h"
#include <globaldefs.h>

struct Struct_020f1f14
{
    unsigned int freeStart_;
    unsigned int freeEnd_;
    unsigned int maybeIsUsable_;
    char unk_c[4];
    unsigned short relatedToPairing_;
    char unk_12[2];
    unsigned int poolBase_;
} extern data_020f1f14[5];

extern Struct_020f1f14* data_020f1ef8[2];
// points to elements in the above array.
// Initially in the order ([4], [3], [0], [2], [1]).
// No idea why
extern Struct_020f1f14* data_020f1f00[5];

struct Struct_0210cf88
{
    unsigned int freeStart_;
    unsigned int freeEnd_;
} extern data_0210cf88;

#pragma optimize_for_size off

// I wasn't able to get this to match, so the file isn't delinked yet
extern "C" unsigned int MaybeAllocateTextureImageVRAM(unsigned int amount, bool paired, unsigned int defaultOffset)
{
    unsigned int chosenOffset = defaultOffset;
    unsigned int alignedSize = amount + 0xf;
    if (amount == 0)
        alignedSize = 0x10;
    else
        alignedSize = (amount + 0xf) & ~0xf;

    if (alignedSize >= 0x7fff0)
        return 0;

    bool success;
    if (paired)
    {
        int pass = 0;
        do
        {
            unsigned int start;
            Struct_020f1f14* secondaryPool;
            Struct_020f1f14* primaryPool;
            
            primaryPool = data_020f1ef8[pass];
            
            if (primaryPool->maybeIsUsable_ == 0)
                continue;
                
            start = primaryPool->freeStart_;
            if (primaryPool->freeEnd_ - start < alignedSize)
                continue;
            
            switch (primaryPool->relatedToPairing_)
            {
            case 0:
                secondaryPool = &data_020f1f14[1];
                break;
            case 3:
                secondaryPool = &data_020f1f14[2];
                break;
            default:
                secondaryPool = NULL;
                break;
            }
            if (secondaryPool->maybeIsUsable_ == 0)
                continue;
            if (secondaryPool->freeEnd_ - secondaryPool->freeStart_ >= (alignedSize >> 1))
            {
                primaryPool->freeStart_ += alignedSize;
                success = true;
                secondaryPool->freeStart_ += (alignedSize >> 1);
                chosenOffset = start + primaryPool->poolBase_;
                goto end;
            }
        } while (pass++, pass < 2);
        success = false;
    }
    else
    {
        int pass = 0;
        do
        {
            Struct_020f1f14* pool;
            pool = data_020f1f00[pass];
            if (pool->maybeIsUsable_ == 0)
                continue;
            if (pool->freeEnd_ - pool->freeStart_ >= alignedSize)
            {
                success = true;
                pool->freeEnd_ = (volatile unsigned int&)pool->freeEnd_ - alignedSize;
                chosenOffset = pool->freeEnd_ + pool->poolBase_;
                goto end;
            }
        } while (pass++, pass < 5);
        success = false;
    }

end:
    if (!success)
        return 0;

    return ((alignedSize >> 4) << 16) | ((chosenOffset << 13) >> 16) | (paired << 31);
}

// What a horrible, horrible way to write a function
unsigned int MaybeAllocateTexturePaletteVRAM(unsigned int amount, bool eightByteAlign, bool unknown)
{  
    unsigned int chosenPosition = 0;
    unsigned int frontPadding;
    unsigned int alignedSize;
    unsigned int totalSpaceNeeded;

    if (amount == 0)
        alignedSize = 8;
    else
        alignedSize = (amount + 7) & ~7;

    // this is 512k, which is better suited as a bound for texture image VRAM
    // but I guess it's general purpose
    if (alignedSize >= 0x7fff8)
        return 0;

    bool success;
    
    if (unknown == true)
    {
        unsigned int start = data_0210cf88.freeStart_;
        if (eightByteAlign)
            frontPadding = (8 - (start & 7)) & 7;
        else
            frontPadding = (16 - (start & 15)) & 15;
        
        totalSpaceNeeded = alignedSize + frontPadding;
        unsigned newFreeStart;
        if (data_0210cf88.freeEnd_ - start < totalSpaceNeeded)
            goto lab_a4;
        newFreeStart = start + totalSpaceNeeded;
        if (!eightByteAlign)
            goto lab_88;
        if (newFreeStart > 0x10000)
        {
            success = false;
            goto end;
        }
    lab_88:
        success = true;
        chosenPosition = data_0210cf88.freeStart_ + frontPadding;
        data_0210cf88.freeStart_ += totalSpaceNeeded;
        goto end;
    lab_a4:
        success = false;
    }
    else
    {
        if (data_0210cf88.freeEnd_ >= alignedSize)
        {
            if (!eightByteAlign)
                frontPadding = (data_0210cf88.freeEnd_ - alignedSize) & 15;
            else
                frontPadding = (data_0210cf88.freeEnd_ - alignedSize) & 7;

            int totalSpaceUsed = alignedSize + frontPadding;

            if (data_0210cf88.freeEnd_ - data_0210cf88.freeStart_ >= alignedSize + frontPadding)
            {
                if (eightByteAlign && data_0210cf88.freeEnd_ > 0x10000)
                {
                    success = false;
                    goto end;
                }
                else
                {
                    success = true;
                    chosenPosition = *(volatile unsigned int*)&data_0210cf88.freeEnd_ - totalSpaceUsed;
                    data_0210cf88.freeEnd_ = chosenPosition;
                    goto end;
                }
            }
        }
        success = false;
    }
end:
    if (!success)
        return 0;

    int mask = (alignedSize >> 3) << 16 | ((chosenPosition << 13) >> 16);
    return mask;
}