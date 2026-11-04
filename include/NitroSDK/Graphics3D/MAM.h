#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DMAM
{
    Nitro3DAnimationSignature signature_;
    uint16_t unk_4;
    uint16_t unk_6;
    Nitro3DNameList tracks_;

    // Each entry in the track describes how to vary the corresponding 
    // material property. The 32 bits are used as follows:
    // bits 0-15: offset, relative to this MAM, to array of samples
    // bits 16-28: index of last frame (i.e. number of frames - 1)
    // bit 29: is constant
    // bits 30-31: log_2(sample rate), only 0,1,2 supported
    // If bit 29 is set, then there is no sample array and instead bits 0-15
    // hold the constant value. Color samples (all except alpha) are 16-bit
    // with only 15 bits used, alpha samples are 8-bit with only 5 bits used.
    struct Track
    {
        uint32_t diffuse_;
        uint32_t ambient_;
        uint32_t reflection_;
        uint32_t emission_;
        uint32_t alpha_;
    };
};