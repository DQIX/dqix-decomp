#include "NitroSDK/Graphics3D/Graphics3D.h"
#include <asmhacks.h>

#define SUBFILE_OFFSETS(nsb) ((uint32_t*)((intptr_t)(nsb) + nsb->headerSize_))

void* NSBXX_GetFirstSubfile(Nitro3DFileHeader* nsbxx)
{
    return (char*)nsbxx + SUBFILE_OFFSETS(nsbxx)[0];
}

Nitro3DTextureSet* NSBXX_GetTEXFile(Nitro3DFileHeader* nsbxx)
{
    const uint32_t* subfiles = SUBFILE_OFFSETS(nsbxx);
    if (nsbxx->numSubfiles_ == 1)
    {
        if (nsbxx->signature_ == SIGNATURE_NSBTX)
            return (Nitro3DTextureSet*)((intptr_t)nsbxx + subfiles[0]);

        return NULL;
    }
    return (Nitro3DTextureSet*)((intptr_t)nsbxx + subfiles[1]);
}

void* NSBXX_GetObjectFromFirstSubfile(Nitro3DFileHeader* nsbxx, unsigned int idx)
{
    if (nsbxx != NULL)
    {
        Nitro3DInnerFileHeader* innerData = (Nitro3DInnerFileHeader*)((intptr_t)nsbxx + SUBFILE_OFFSETS(nsbxx)[0]);

        intptr_t namelistPtr = (intptr_t)&innerData->nameList;
        uint32_t* pInnerOffset;
        if (namelistPtr != NULL && idx < innerData->nameList.numEntries_)
        {
            uint16_t stride = *(uint16_t*)(namelistPtr + innerData->nameList.offsetToDataStart_);
            pInnerOffset = (uint32_t*)(namelistPtr + innerData->nameList.offsetToDataStart_ + 4 + stride * idx);
        }
        else
        {
            pInnerOffset = NULL;
        }

        if (pInnerOffset != NULL)
        {
            return (char*)innerData + *pInnerOffset;
        }
    }
    return NULL;
}

const char* NSBXX_PatternAnimation_GetTextureName(Nitro3DMPT* anim, unsigned int idx)
{
    if (anim != NULL && idx < anim->numTextureNames_)
    {
        return (const char*)((intptr_t)anim + anim->textureNamesOffset_ + idx * 0x10);
    }
    return NULL;
}

const char* NSBXX_PatternAnimation_GetPaletteName(Nitro3DMPT* anim, unsigned int idx)
{
    if (anim != NULL && idx < anim->numPaletteNames_)
    {
        return (const char*)((intptr_t)anim + anim->paletteNamesOffset_ + idx * 0x10);
    }
    return NULL;
}

Nitro3DMPT::Track::Keyframe* NSBXX_PatternAnimation_GetKeyframe(
    Nitro3DMPT* anim, uint16_t track, uint16_t frameTime)
{
    Nitro3DMPT::Track* pTrack = NSBXX_PatternAnimation_GetTrack(anim, track);
    Nitro3DMPT::Track::Keyframe* keyframes =
        (Nitro3DMPT::Track::Keyframe*)((intptr_t)anim + pTrack->keyframeArrayOffset_);
    
    // Start with a heuristic for the index
    unsigned int index = (unsigned int)(pTrack->maybeSpeed_4_ * frameTime) >> 12u;

    while (index != 0 && keyframes[index].frameTime_ >= frameTime)
        index--;

    int maxKeyframes = pTrack->numKeyframes_;
    while (index + 1 < maxKeyframes && (keyframes + 1)[index].frameTime_ <= frameTime)
        index++;

    return &keyframes[index];
}

Nitro3DMPT::Track* NSBXX_PatternAnimation_GetTrack(
    Nitro3DMPT* anim, unsigned int track)
{
    intptr_t nameList = (intptr_t)&anim->tracks_;
    if (nameList != 0 && track < anim->tracks_.numEntries_)
    {
        uint16_t stride = *(uint16_t*)(nameList + anim->tracks_.offsetToDataStart_);
        intptr_t ret = nameList + anim->tracks_.offsetToDataStart_ + 4 + stride * track;
        return (Nitro3DMPT::Track*)ret;
    }
    return NULL;
}