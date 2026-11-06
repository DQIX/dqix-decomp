#include "NitroSDK/Graphics3D/Internal.h"

const char* Nitro3Di_GetPatternAnimationTextureName(Nitro3DMPT* anim, unsigned int idx)
{
    if (anim != NULL && idx < anim->numTextureNames)
    {
        return (const char*)((intptr_t)anim + anim->textureNamesOffset + idx * 0x10);
    }
    return NULL;
}

const char* Nitro3Di_GetPatternAnimationPaletteName(Nitro3DMPT* anim, unsigned int idx)
{
    if (anim != NULL && idx < anim->numPaletteNames)
    {
        return (const char*)((intptr_t)anim + anim->paletteNamesOffset + idx * 0x10);
    }
    return NULL;
}

Nitro3DMPT::Track::Keyframe* Nitro3Di_GetPatternAnimationKeyframe(
    Nitro3DMPT* anim, uint16_t track, uint16_t frameTime)
{
    Nitro3DMPT::Track* pTrack = Nitro3Di_GetPatternAnimationTrack(anim, track);
    Nitro3DMPT::Track::Keyframe* keyframes =
        (Nitro3DMPT::Track::Keyframe*)((intptr_t)anim + pTrack->keyframeArrayOffset);
    
    // Start with a heuristic for the index
    unsigned int index = (unsigned int)(pTrack->maybeSpeed_4 * frameTime) >> 12u;

    while (index != 0 && keyframes[index].frameStartTime >= frameTime)
        index--;

    int maxKeyframes = pTrack->numKeyframes;
    while (index + 1 < maxKeyframes && (keyframes + 1)[index].frameStartTime <= frameTime)
        index++;

    return &keyframes[index];
}

Nitro3DMPT::Track* Nitro3Di_GetPatternAnimationTrack(Nitro3DMPT* anim, unsigned int track)
{
    return anim->tracks.GetEntryByIndex<Nitro3DMPT::Track>(track);
}