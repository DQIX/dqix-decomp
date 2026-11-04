#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DMAT
{
    Nitro3DAnimationSignature signature;
    char unk_4[4];
    Nitro3DNameList tracks;

    // For each parameter (scaleX, scaleY etc) the metadata packs the following
    // info into 32 bits:
    // bits 0-15: end frame index, i.e. #frames - 1
    // bit 28: are samples 16-bit? (not used for angle data)
    // bit 29: is track constant? if so, corresponding offset entry holds the const value instead
    // bits 30-31: log_2(sampling rate), either 0,1,2 for every 1,2,4 frames
    //
    // offset holds the offset from the start of the MAT to the array of relevant samples
    struct Track
    {
        uint32_t scaleXMetadata;
        uint32_t scaleXOffset;
        uint32_t scaleYMetadata;
        uint32_t scaleYOffset;
        uint32_t rotationMetadata;
        uint32_t rotationOffset;
        uint32_t translateXMetadata;
        uint32_t translateXOffset;
        uint32_t translateYMetadata;
        uint32_t translateYOffset;
    };
};