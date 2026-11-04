#pragma once

#include "Common.h"
#include "Animation.h"

struct Nitro3DMAT
{
    Nitro3DAnimationSignature signature_;
    uint16_t unk_4;
    uint16_t unk_6;
    Nitro3DNameList tracks_;

    struct Track
    {
        uint32_t unk_0;
        uint32_t unk_4;
        uint32_t unk_8;
        uint32_t unk_c;
        uint32_t unk_10;
        uint32_t unk_14;
        uint32_t unk_18;
        uint32_t unk_1c;
        uint32_t unk_20;
        uint32_t unk_24;
    };
};