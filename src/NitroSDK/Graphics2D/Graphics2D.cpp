#include "NitroSDK/Graphics2D/Graphics2D.h"
#include <globaldefs.h>

#define SIGNATURE_ABNK 0x41424e4b

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

void PrepareABNK(Nitro2DAnimationBank* ptr);

CBool InternalGetAnimationBank(void* fileData, Nitro2DAnimationBank** output)
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

void PrepareABNK(Nitro2DAnimationBank* ptr)
{
    FixPointerOffsetRight(&ptr->pAnimBlocks_4, ptr);
    FixPointerOffsetRight(&ptr->pFrameBlocks_8, ptr);
    FixPointerOffsetRight(&ptr->pFrameData_c, ptr);

    unsigned short blockIndex;
    unsigned short frameIndex;
    uint16_t* frameData = ptr->pFrameData_c;
    Nitro2DAnimationBank::AnimationBlock* animBlocks = ptr->pAnimBlocks_4;
    Nitro2DAnimationBank::FrameBlock* frameBlocks = ptr->pFrameBlocks_8;

    for (blockIndex = 0; ptr->numAnimBlocks_0 > blockIndex; blockIndex++)
    {
        FixPointerOffsetLeft(&animBlocks[blockIndex].unknown_c, frameBlocks);
        for (frameIndex = 0; animBlocks[blockIndex].numFrames_0 > frameIndex; frameIndex++)
        {
            FixPointerOffsetLeft(&animBlocks[blockIndex].unknown_c[frameIndex].unknown_0, frameData);
        }
    }

    if (ptr->unknown_14 == NULL)
        return;

    FixPointerOffsetRight(&ptr->unknown_14, ptr);
    Nitro2DAnimationBank::Subsubstruct* subsub = &ptr->unknown_14->unknown_8;
    FixPointerOffsetRight(&subsub->unknown_4, subsub);
    for (unsigned int i = 0; i < subsub->unknown_0; i++)
    {
        unsigned int j;
        Nitro2DAnimationBank::Subsubsub* sss = &subsub->unknown_4[i];
        FixPointerOffsetRight(&sss->unknown_4, subsub);
        FixPointerOffsetRight(&sss->unknown_8, subsub);
        for (j = 0; j < sss->unknown_0; j++)
        {
            FixPointerOffsetRight(&sss->unknown_8[j], subsub);
        }
    }
}

Nitro2DInnerFileHeader* Nitro2D_GetInnerFileBySignature(void* fileData, uint32_t signature)
{
    Nitro2DFileHeader* header = (Nitro2DFileHeader*)fileData;
    Nitro2DInnerFileHeader* innerFile = header->GetFirstSubfile();
    for (unsigned short i = 0; header->numSubfiles > i; i++)
    {
        if (innerFile->signature == signature)
            return innerFile;
        innerFile = innerFile->Next();
    }
    return NULL;
}