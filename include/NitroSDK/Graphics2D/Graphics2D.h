#pragma once

#include <std_library_functions.h>

struct Nitro2DInnerFileHeader;

struct Nitro2DFileHeader
{
    uint32_t signature;
    uint16_t byteOrderMark;
    uint16_t version;
    uint32_t fileSize;
    uint16_t firstSubfileOffset;
    uint16_t numSubfiles;

    Nitro2DInnerFileHeader* GetFirstSubfile()
    {
        return (Nitro2DInnerFileHeader*)((char*)this + firstSubfileOffset);
    }
};

struct Nitro2DInnerFileHeader
{
    uint32_t signature;
    uint32_t fileSize;

    Nitro2DInnerFileHeader* Next()
    {
        return (Nitro2DInnerFileHeader*)((char*)this + fileSize);
    }

    void* GetInterior()
    {
        return this + 1;
    }
};

// signature 'ABNK'
struct Nitro2DAnimationBank
{
    struct FrameBlock;
    struct AnimationBlock
    {
        uint16_t numFrames_0;
        uint16_t unknown_2;
        uint16_t unknown_4;
        uint16_t unknown_6;
        uint32_t unknown_8;
        FrameBlock* unknown_c;
    };

    struct FrameBlock
    {
        uint16_t* unknown_0;
        uint16_t unknown_4;
        uint16_t unknown_6;
    };

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

    uint16_t numAnimBlocks_0;
    uint16_t numFrameBlocks_2;
    AnimationBlock* pAnimBlocks_4;
    FrameBlock* pFrameBlocks_8;
    uint16_t* pFrameData_c;
    char unk_10[4];
    Substruct14* unknown_14;
};

CBool Nitro2D_GetAnimationBank(void* fileData, Nitro2DAnimationBank** output);

Nitro2DInnerFileHeader* Nitro2D_GetInnerFileBySignature(void* fileData, uint32_t signature);