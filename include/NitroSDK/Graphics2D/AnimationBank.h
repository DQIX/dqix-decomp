#pragma once

#include "Common.h"

// signature 'ABNK'
struct Nitro2DAnimationBank
{
    struct FrameBlock;
    struct AnimationBlock
    {
        uint16_t numFrames;
        uint16_t unknown_2;
        uint16_t unknown_4;
        uint16_t unknown_6;
        // 1 = stop at end, 2 = loop (based on OBJSpriteSet)
        uint32_t animationType;
        FrameBlock* pFrames;
    };

    struct FrameBlock
    {
        uint16_t* metatileIndices; // see cell bank
        uint16_t duration;
        uint16_t unknown_6;
    };

    // some nonsense, never gets used and has no GBATEK documentation
    struct Subsubsub
    {
        unsigned short unknown_0;
        char unk_2[2];
        void* unknown_4;
        void** unknown_8;
    };

    struct Subsubstruct
    {
        uint16_t unknown_0;
        char unk_2[2];
        Subsubsub* unknown_4;
    };

    struct Substruct14
    {
        char unk_0[8];
        Subsubstruct unknown_8;
    };

    uint16_t numAnimBlocks;
    uint16_t numFrameBlocks;
    AnimationBlock* pAnimBlocks;
    FrameBlock* pFrameBlocks;
    uint16_t* pFrameData;
    char unk_10[4];
    Substruct14* unknown_14;
};

CBool Nitro2D_GetAnimationBank(void* fileData, Nitro2DAnimationBank** output);
Nitro2DAnimationBank::AnimationBlock* Nitro2D_AnimationBankGetAnimationBlock(Nitro2DAnimationBank*, unsigned int idx);