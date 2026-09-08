#include "Graphics/NSBXX/NSBXX.h"
#include <asmhacks.h>

#pragma optimize_for_size off

#define SUBFILE_OFFSETS(nsb) ((uint32_t*)((intptr_t)(nsb) + nsb->headerSize_))

void* NSBXX_GetFirstSubfile(NSBXXContainer* nsbxx)
{
    return (char*)nsbxx + SUBFILE_OFFSETS(nsbxx)[0];
}

NSBXXTex* NSBXX_GetTEXFile(NSBXXContainer* nsbxx)
{
    const uint32_t* subfiles = SUBFILE_OFFSETS(nsbxx);
    if (nsbxx->numSubfiles_ == 1)
    {
        if (nsbxx->signature_ == SIGNATURE_NSBTX)
            return (NSBXXTex*)((intptr_t)nsbxx + subfiles[0]);

        return NULL;
    }
    return (NSBXXTex*)((intptr_t)nsbxx + subfiles[1]);
}

void* NSBXX_GetObjectFromFirstSubfile(NSBXXContainer* nsbxx, unsigned int idx)
{
    if (nsbxx != NULL)
    {
        NSBXXInnerFileCommon* innerData = (NSBXXInnerFileCommon*)((intptr_t)nsbxx + SUBFILE_OFFSETS(nsbxx)[0]);

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

const char* NSBXX_PatternAnimation_GetTextureName(NSBXXAnimationMPT* anim, unsigned int idx)
{
    if (anim != NULL && idx < anim->numTextureNames_)
    {
        DECLARE_ASM_NOP();
        return (const char*)((intptr_t)anim + anim->textureNamesOffset_ + idx * 0x10);
    }
    return NULL;
}

const char* NSBXX_PatternAnimation_GetPaletteName(NSBXXAnimationMPT* anim, unsigned int idx)
{
    if (anim != NULL && idx < anim->numPaletteNames_)
    {
        DECLARE_ASM_NOP();
        return (const char*)((intptr_t)anim + anim->paletteNamesOffset_ + idx * 0x10);
    }
    return NULL;
}

NSBXXAnimationMPT::Track::Keyframe* NSBXX_PatternAnimation_GetKeyframe(
    NSBXXAnimationMPT* anim, uint16_t track, uint16_t frameTime)
{
    NSBXXAnimationMPT::Track* pTrack = NSBXX_PatternAnimation_GetTrack(anim, track);
    NSBXXAnimationMPT::Track::Keyframe* keyframes =
        (NSBXXAnimationMPT::Track::Keyframe*)((intptr_t)anim + pTrack->keyframeArrayOffset_);
    
    // Start with a heuristic for the index
    unsigned int index = (unsigned int)(pTrack->maybeSpeed_4_ * frameTime) >> 12u;

    while (index != 0 && keyframes[index].frameTime_ >= frameTime)
        index--;

    int maxKeyframes = pTrack->numKeyframes_;
    while (index + 1 < maxKeyframes && (keyframes + 1)[index].frameTime_ <= frameTime)
        index++;

    return &keyframes[index];
}

NSBXXAnimationMPT::Track* NSBXX_PatternAnimation_GetTrack(
    NSBXXAnimationMPT* anim, unsigned int track)
{
    intptr_t nameList = (intptr_t)&anim->tracks_;
    if (nameList != 0 && track < anim->tracks_.numEntries_)
    {
        uint16_t stride = *(uint16_t*)(nameList + anim->tracks_.offsetToDataStart_);
        intptr_t ret = nameList + anim->tracks_.offsetToDataStart_ + 4 + stride * track;
        return (NSBXXAnimationMPT::Track*)ret;
    }
    return NULL;
}