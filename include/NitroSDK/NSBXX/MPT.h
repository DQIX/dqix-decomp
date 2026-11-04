#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DMPT
{
    Nitro3DAnimationSignature signature_;
    uint16_t numFrames_;
    uint8_t numTextureNames_;
    uint8_t numPaletteNames_;
    uint16_t textureNamesOffset_;
    uint16_t paletteNamesOffset_;

    Nitro3DNameList tracks_;

    struct Track
    {
        uint16_t numKeyframes_;
        uint16_t unk_2; // I thought keyframes was u32 but it's loaded as u16
        int16_t maybeSpeed_4_; // I imagine this is a 1.3.12 fixed point, it took values like 0x0555 (1/3)
        uint16_t keyframeArrayOffset_;

        struct Keyframe
        {
            uint16_t frameTime_;
            uint8_t textureIdx_;
            uint8_t paletteIdx_;
        };
    };
};