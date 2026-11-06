#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DMPT
{
    Nitro3DAnimationSignature signature;
    uint16_t numFrames;
    uint8_t numTextureNames;
    uint8_t numPaletteNames;
    uint16_t textureNamesOffset;
    uint16_t paletteNamesOffset;

    Nitro3DNameList tracks;

    struct Track
    {
        uint16_t numKeyframes; // not u32 like gbatek says
        char unk_2[2];
        // (only?) used as a heuristic when looking up keyframe
        fix16_t maybeSpeed_4;
        uint16_t keyframeArrayOffset;

        struct Keyframe
        {
            uint16_t frameStartTime;
            uint8_t textureIdx;
            uint8_t paletteIdx;
        };
    };
};