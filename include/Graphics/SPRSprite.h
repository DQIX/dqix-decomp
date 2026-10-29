#pragma once

#include <globaldefs.h>
#include "Vector.h"
#include "../Memory/SafeAllocator.h"

// sizeof == 0x88.
// A 2D (animated) sprite rendered using the 3D engine.
// Has an inline constructor that gets emitted at 0201c0e8 and a destructor
// emitted at 0201c108 (usa), calling Reset() and Release() respectively
class SPRSprite
{
public:
    typedef char AnimationName[30];
    // Has an inline constructor and destructor that each call Reset(),
    // doesn't appear to be emitted anywhere
    struct Names
    {
        int count;
        AnimationName* array;

        void Reset();
        void Allocate(int count, SafeAllocator* alloc);
        // gets the index in the array matching the name, or
        // 0 if no match found
        int Find(const char* str);
    };

    // sizeof == 0xc
    struct Frame
    {
        int nextFrameIdx;
        int duration;
        int textureIndex;
    };

    // sizeof == 8
    struct SpriteAnimation
    {
        unsigned int numFrames;
        Frame* frames;
    };

    // sizeof == 8
    struct TexturePiece
    {
        unsigned int vramOffset;
        // log_2(width/8) in low 4 bits, log_2(height/8) in upper 4
        unsigned short logDimensions;
        // x in low 8 bits, y in high 8.
        // y = 0 here means aligned to the *bottom* of the texture
        // (happens when the file holds a y-value of (tex height) - (piece height))
        unsigned short position;
    };

    struct Texture
    {
        unsigned char width;
        unsigned char height;
        unsigned char numPieces;
        char pad_3[1];
        TexturePiece* pieces;
    };

    int polygonID_;
    // probably only supports formats 3 (16-color) and 4 (256-color)?
    unsigned short textureFormat_;
    unsigned short numTextures_;
    Texture* textures_;
    SpriteAnimation* animations_;
    unsigned int paletteVRAMOffset_;
    Names animationNames_;
    Vector3fix position_;
    Vector3fix rotation_;
    Vector3fix scale_;
    char unk_40[0x30];
    int currentFrameIdx_;
    int animationIdx_;
    int frameStartTimestamp_;
    unsigned int currentFrameDuration_;
    unsigned short tint_;
    short alpha_;
    unsigned char initialized_ : 1;
    unsigned char unknown_84_1_ : 1;
    unsigned char unknown_84_2_ : 1;
    unsigned char pad_84_upper : 5;
    char pad_85[3];

    void Reset();
    void Release();

    void Update();

    void DrawAnimatedWithRotation(bool transparent0);
    void DrawAnimatedNoRotation(bool transparent0);
    
    void DrawTextureScaleRotateTranslate(int index, bool transparent0);
    void DrawTextureScaleTranslate(int index, bool transparent0);
    void DrawTextureStatic(int index, bool transparent0);
    void DrawTexturePiece(TexturePiece* image, bool transparent0);

    void SetAnimation(int idx);
    void SetAnimationRandomTime(const char* name);

    unsigned char GetCurrentTextureHeight() const;
    unsigned char GetCurrentTextureWidth() const;

    unsigned char GetTextureHeight(int idx) const;
    unsigned char GetTextureWidth(int idx) const;

    // loads from data/ani/<name>
    void LoadAnimationFromFile(const char* name, SafeAllocator* alloc);
    void LoadFromSPR(const void* data, unsigned int length, SafeAllocator* alloc);
    void LoadFromSPR(const void* data, SafeAllocator* alloc);
    void AllocateTextureImage(TexturePiece* into, const void* data, unsigned int length, SafeAllocator* alloc);
    void AllocateTexturePalette(const void* data, unsigned int length, SafeAllocator* alloc);

    void CloneTo(SPRSprite* out);
};