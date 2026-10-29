#include "Graphics/SPRSprite.h"
#include "Graphics/VRAMStaging.h"
#include "NitroSDK/HWRegisters.h"
#include "NitroSDK/VRAMAllocation.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"

#if defined(jpn)
#define data_02114e00 data_02114aa0
#define data_02114e50 data_02114af0

#define func_020311f0 func_02030d28
#define func_02031234 func_02030d6c
#define func_02031278 func_02030db0
#define func_020429d4 func_02042e94
#endif

extern "C"
{
    // submit x-rotation to fifo
    void func_020311f0(fix32_t);
    // submit y-rotation to fifo
    void func_02031234(fix32_t);
    // submit z-rotation to fifo
    void func_02031278(fix32_t);

    // gets the staging buffer used by textmanager
    void* func_020429d4();
}

extern bool data_02114e00;
extern int data_02114e50; // frame counter

static inline void WriteVertex(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_VERTEX_16 = (unsigned short)x | ((unsigned short)y << 16);
    GXFIFO_VERTEX_16 = (unsigned short)z;
}

static inline void WriteTexCoords(fix32_t x, fix32_t y)
{
    short xFixed4 = x >> 8;
    short yFixed4 = y >> 8;
    GXFIFO_VERTEX_TEXCOORD = (unsigned short)xFixed4 | ((unsigned short)yFixed4 << 16);
}

static inline void WriteTranslate(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_MATRIX_TRANSLATE = x;
    GXFIFO_MATRIX_TRANSLATE = y;
    GXFIFO_MATRIX_TRANSLATE = z;
}

static inline void WriteScale(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_MATRIX_SCALE = x;
    GXFIFO_MATRIX_SCALE = y;
    GXFIFO_MATRIX_SCALE = z;
}

void SPRSprite::Reset()
{
    polygonID_ = 0;
    numTextures_ = 0;
    textures_ = NULL;
    animations_ = NULL;
    animationNames_.count = 0;
    textureFormat_ = 3; // 4bpp format
    paletteVRAMOffset_ = 0;
    initialized_ = false;
    animationIdx_ = frameStartTimestamp_ = currentFrameDuration_ = 0;
    currentFrameIdx_ = 0;
    position_.x = 0;
    position_.y = 0;
    position_.z = 0;
    rotation_.x = 0;
    rotation_.y = 0;
    rotation_.z = 0;
    scale_.x = 1 << 12;
    scale_.y = 1 << 12;
    scale_.z = 1 << 12;
    alpha_ = 31;
    tint_ = 0x7fff;
    unknown_84_1_ = false;
    unknown_84_2_ = false;
}

void SPRSprite::Release()
{
    if (initialized_)
        Reset();
}

void SPRSprite::Update()
{
    if (!initialized_ || !unknown_84_1_)
        return;

    if (data_02114e00)
    {
        frameStartTimestamp_ = data_02114e50;
        return;
    }

    unsigned int timeSpentOnFrame = data_02114e50 - frameStartTimestamp_;
    if (currentFrameDuration_ >= timeSpentOnFrame)
        return;

    unsigned int oldFrameIdx = currentFrameIdx_;
    Frame* frame = &animations_[animationIdx_].frames[oldFrameIdx];
    currentFrameIdx_ = frame->nextFrameIdx;
    frameStartTimestamp_ = data_02114e50;
    currentFrameDuration_ = frame->duration;
}

void SPRSprite::DrawAnimatedWithRotation(bool transparent0)
{
    if (!initialized_ || !unknown_84_1_ || alpha_ <= 0)
        return;

    SpriteAnimation* animation = &animations_[animationIdx_];
    Frame* frame = &animation->frames[currentFrameIdx_];
    GXFIFO_MATRIX_PUSH = 0;
    WriteTranslate(position_.x, position_.y, position_.z);
    WriteScale(scale_.x, scale_.y, scale_.z);

    if (rotation_.z != 0)
        func_02031278(rotation_.z);
    if (rotation_.y != 0)
        func_02031234(rotation_.y);
    if (rotation_.x != 0)
        func_020311f0(rotation_.x);

    DrawTextureScaleRotateTranslate(frame->textureIndex, transparent0);
    GXFIFO_MATRIX_POP = 1;
}

void SPRSprite::DrawAnimatedNoRotation(bool transparent0)
{
    if (!initialized_ || !unknown_84_1_ || alpha_ <= 0 || animations_ == NULL)
        return;

    SpriteAnimation* animation = &animations_[animationIdx_];
    Frame* frame = &animation->frames[currentFrameIdx_];
    GXFIFO_MATRIX_PUSH = 0;
    DrawTextureScaleTranslate(frame->textureIndex, transparent0);
    GXFIFO_MATRIX_POP = 1;
}

void SPRSprite::DrawTextureScaleRotateTranslate(int index, bool transparent0)
{
    if (!initialized_ || !unknown_84_1_ || alpha_ <= 0)
        return;

    Texture* sub8 = &textures_[index];

    if (sub8->pieces == NULL || sub8->numPieces == 0)
        return;

    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_DIFFUSE_AMBIENT = 0x7fffffff;
    GXFIFO_SPECULAR_EMISSION = 0x4210;
    fix32_t x = position_.x - (sub8->width >> 1) * scale_.x;
    WriteTranslate(x, position_.y, position_.z);
    func_02031234(rotation_.y);
    WriteScale(scale_.x, scale_.y, scale_.z);

    TexturePiece* image = sub8->pieces;
    for (int i = 0; i < sub8->numPieces; i++, image++)
    {
        DrawTexturePiece(image, transparent0);
    }
    GXFIFO_MATRIX_POP = 1;
}

void SPRSprite::DrawTextureScaleTranslate(int index, bool transparent0)
{
    if (!initialized_ || !unknown_84_1_ || alpha_ <= 0)
        return;

    Texture* sub8 = &textures_[index];

    if (sub8->pieces == NULL || sub8->numPieces == 0)
        return;

    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_DIFFUSE_AMBIENT = 0x7fffffff;
    GXFIFO_SPECULAR_EMISSION = 0x4210;
    fix32_t y = position_.y + sub8->height * scale_.y;
    WriteTranslate(position_.x, y, position_.z);
    WriteScale(scale_.x, -scale_.y, scale_.z);

    TexturePiece* image = sub8->pieces;
    for (int i = 0; i < sub8->numPieces; i++, image++)
    {
        DrawTexturePiece(image, transparent0);
    }
    GXFIFO_MATRIX_POP = 1;
}

void SPRSprite::DrawTextureStatic(int index, bool transparent0)
{
    if (!initialized_ || !unknown_84_1_ || alpha_ <= 0)
        return;

    Texture* texture = &textures_[index];

    if (texture->pieces == NULL || texture->numPieces == 0)
        return;

    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_DIFFUSE_AMBIENT = 0x7fffffff;
    GXFIFO_SPECULAR_EMISSION = 0x4210;
    fix32_t value = texture->width << 12;
    WriteTranslate(-value >> 1, 0, 0);

    TexturePiece* image = texture->pieces;
    for (int i = 0; i < texture->numPieces; i++, image++)
    {
        DrawTexturePiece(image, transparent0);
    }
    GXFIFO_MATRIX_POP = 1;
}

void SPRSprite::DrawTexturePiece(TexturePiece *image, bool transparent0)
{
    GXFIFO_MATRIX_PUSH = 0;
    int logWidth = image->logDimensions & 0xf;
    int logHeight = (image->logDimensions & 0xf0) >> 4;
    fix32_t width = 8 << (logWidth + 12);
    fix32_t height = 8 << (logHeight + 12);
    fix32_t posX = (image->position & 0xff) << 12;
    fix32_t posY = ((image->position & 0xff00) >> 8) << 12;
    int textureFormat = textureFormat_;
    textureFormat <<= 26;
    GXFIFO_TEXIMAGE_PARAMS = (image->vramOffset >> 3) | 
        textureFormat | (logWidth << 20) | (logHeight << 23) | (transparent0 << 29);
    GXFIFO_PALETTE_BASE = paletteVRAMOffset_ >> (4 - (textureFormat_ == 2));
    // show front & back face, enable fog
    int alphaMask = (alpha_ & 0x1f);
    alphaMask <<= 16;
    GXFIFO_POLYGON_ATTRIBUTES = 0x80c0 | (polygonID_ << 24) | alphaMask;

    WriteTranslate(posX, posY, 0);
    WriteScale(width, height, 1 << 12);

    GXFIFO_POLYGON_BEGIN = 1;
    GXFIFO_VERTEX_COLOR = tint_;

    GXFIFO_VERTEX_TEXCOORD = 0;
    WriteVertex(0, 1 << 12, 0);

    WriteTexCoords(width, 0);
    WriteVertex(1 << 12, 1 << 12, 0);

    WriteTexCoords(width, height);
    WriteVertex(1 << 12, 0, 0);

    WriteTexCoords(0, height);
    WriteVertex(0, 0, 0);

    GXFIFO_POLYGON_END = 0;
    GXFIFO_MATRIX_POP = 1;

}

void SPRSprite::SetAnimation(int idx)
{
    if (!initialized_ || animationNames_.count <= 0)
        return;
    if (animationIdx_ == idx || idx == 0)
        return;

    currentFrameIdx_ = 0;
    animationIdx_ = idx;
    frameStartTimestamp_ = data_02114e50;
    currentFrameDuration_ = animations_[animationIdx_].frames[currentFrameIdx_].duration;
}

void SPRSprite::SetAnimationRandomTime(const char* name)
{
    if (!initialized_)
        return;
    int targetIndex = animationNames_.Find(name);
    if (animationIdx_ == targetIndex || targetIndex == 0)
        return;

    animationIdx_ = targetIndex;
    currentFrameIdx_ = 0;
    frameStartTimestamp_ = data_02114e50;
    currentFrameDuration_ = animations_[animationIdx_].frames[currentFrameIdx_].duration;
    // subtract from start timestamp: advances how far we're in the current frame
    frameStartTimestamp_ -= rand() % currentFrameDuration_;   
}

unsigned char SPRSprite::GetCurrentTextureHeight() const
{
    if (!initialized_)
        return 0;
    int idx = animations_[animationIdx_].frames[currentFrameIdx_].textureIndex;
    return textures_[idx].height;
}

unsigned char SPRSprite::GetCurrentTextureWidth() const
{
    if (!initialized_)
        return 0;
    int idx = animations_[animationIdx_].frames[currentFrameIdx_].textureIndex;
    return textures_[idx].width;
}

unsigned char SPRSprite::GetTextureHeight(int idx) const
{
    if (!initialized_)
        return 0;
    return textures_[idx].height;
}

unsigned char SPRSprite::GetTextureWidth(int idx) const
{
    if (!initialized_)
        return 0;
    return textures_[idx].width;
}

struct TextureFileData
{
    unsigned short width;
    unsigned short height;
    unsigned int numPieces;
};

struct TexturePieceFileData
{
    unsigned short x;
    unsigned short y;
    unsigned short logWidth;
    unsigned short logHeight;
};

void SPRSprite::LoadAnimationFromFile(const char* filename, SafeAllocator* alloc)
{
    if (filename == NULL)
        return;
    if (initialized_)
    {
        Release();
        initialized_ = false;
    }
    if (*(filename + strlen(filename) - 1) != 'r') // e.g. file extension is .spr
        return;

    char fullPath[0x80] = {0};
    sprintf(fullPath, "/data/ani/%s", filename);
    void* staging = func_020429d4();
    BackgroundLoader::AddLockGlobal();

    if (LoadFileIntoMemory(fullPath, staging, NULL) != NULL)
        LoadFromSPR(staging, alloc);

    BackgroundLoader::RemoveLockGlobal();
}

void SPRSprite::LoadFromSPR(const void* data, unsigned int length, SafeAllocator* alloc)
{
    LoadFromSPR(data, alloc);
}

void SPRSprite::LoadFromSPR(const void* data, SafeAllocator* alloc)
{
    if (data == NULL)
        return;

    if (initialized_)
    {
        Release();
        initialized_ = false;
    }

    const unsigned char* dataBytes = (const unsigned char*)data;
    memcpy(&numTextures_, dataBytes, 2);
    dataBytes += 2;
    memcpy(&textureFormat_, dataBytes, 2);
    dataBytes += 2;

    textures_ = (Texture*)alloc->Allocate(numTextures_ * sizeof(Texture));
    Texture* loopTexture = textures_;
    for (int i = 0; i < numTextures_; i++, loopTexture++)
    {
        // beware UB from type punning
        const TextureFileData* textureData = (const TextureFileData*)dataBytes;
        dataBytes += sizeof(TextureFileData);
        loopTexture->width = textureData->width;
        loopTexture->height = textureData->height;
        loopTexture->numPieces = textureData->numPieces;
        loopTexture->pieces = (TexturePiece*)alloc->Allocate(loopTexture->numPieces * 8);
        TexturePiece* loopPiece = loopTexture->pieces;
        for (int j = 0; j < loopTexture->numPieces; loopPiece++, j++)
        {
            const TexturePieceFileData* pieceData = (const TexturePieceFileData*)dataBytes;
            dataBytes += sizeof(TexturePieceFileData);
            int bppRightShift = 0; // 0 for 8bpp, 1 for 4bpp
            unsigned int numPixels;
            unsigned int pieceWidth = 8 << pieceData->logWidth;
            unsigned int pieceHeight = 8 << pieceData->logHeight;
            if (textureFormat_ == 3)
            {
                bppRightShift = 1;
                numPixels = pieceWidth * pieceHeight;
                AllocateTextureImage(loopPiece, dataBytes, numPixels / 2, alloc); 
            }
            else
            {
                numPixels = pieceWidth * pieceHeight;
                AllocateTextureImage(loopPiece, dataBytes, numPixels, alloc);
            }
            dataBytes += (numPixels >> bppRightShift);
            loopPiece->logDimensions = (pieceData->logHeight << 4) | pieceData->logWidth;
            // inconsistency with how y is stored: in the file, y = 0 means the top of
            // the texture, here it means the bottom
            loopPiece->position = pieceData->x | ((loopTexture->height - pieceData->y - pieceHeight) << 8);
        }
    }

    unsigned int sprCount;
    memcpy(&sprCount, dataBytes, 4);
    dataBytes += 4;

    if (sprCount <= 16)
        AllocateTexturePalette(dataBytes, 32, alloc);
    else if (sprCount <= 256)
        AllocateTexturePalette(dataBytes, 512, alloc);
    else
        AllocateTexturePalette(dataBytes, sprCount, alloc);

    dataBytes += sprCount * 2;
    dataBytes += 30;
    memcpy(&animationNames_.count, dataBytes, 4);
    dataBytes += 4;

    animationNames_.Allocate(animationNames_.count, alloc);
    if (animationNames_.count > 0)
    {
        for (int i = 0; i < animationNames_.count; i++)
        {
            memcpy(animationNames_.array[i], dataBytes, 30);
            dataBytes += 30;
        }
        animations_ = (SpriteAnimation*)alloc->Allocate(animationNames_.count * sizeof(SpriteAnimation));
        SpriteAnimation* anim = animations_;
        for (int i = 0; i < animationNames_.count; i++, anim++)
        {
            memcpy(&anim->numFrames, dataBytes, 4);
            dataBytes += 4;
            anim->frames = (Frame*)alloc->Allocate(anim->numFrames * sizeof(Frame));
            Frame* frame = anim->frames;

            int* nextIndexArray = (int*)dataBytes;
            dataBytes += anim->numFrames * 4;
            int* durationArray = (int*)dataBytes;
            dataBytes += anim->numFrames * 4;
            int* textureIndexArray = (int*)dataBytes;
            dataBytes += anim->numFrames * 4;

            for (int j = 0; j < anim->numFrames; frame++, j++)
            {
                frame->nextFrameIdx = *nextIndexArray;
                frame->duration = *durationArray;
                frame->textureIndex = *textureIndexArray;
                nextIndexArray++;
                durationArray++;
                textureIndexArray++;
            }
        }
    }

    initialized_ = true;
    unknown_84_1_ = true;
    if (animations_ != NULL)
    {
        if (animations_->numFrames == 0 && animationNames_.count == 1)
            SetAnimation(0);
        else
            SetAnimation(1);
    }
    else
        SetAnimation(0);
}

void SPRSprite::AllocateTextureImage(TexturePiece* into, const void* data, unsigned int length, SafeAllocator* alloc)
{
    unsigned int handle = FrameAllocateTextureImageVRAM(length, false, 0);
    unsigned int offset = (handle & 0xffff) << 3;
    bool allocStagingSpace = true;
    if (unknown_84_2_ && alloc != NULL)
    {
        void* stagingSpace = alloc->Allocate(length);
        if (stagingSpace != NULL)
        {
            memcpy(stagingSpace, data, length);
            data = stagingSpace;
            allocStagingSpace = false;
        }
    }
    StageMemoryToVRAM(VRAMSubregion_TextureImage, data, offset, length, false, allocStagingSpace);
    into->vramOffset = offset;
}

void SPRSprite::AllocateTexturePalette(const void* data, unsigned int length, SafeAllocator* alloc)
{
    unsigned int handle = FrameAllocateTexturePaletteVRAM(length, false, 0);
    unsigned int offset = (handle & 0xffff) << 3;
    bool allocStagingSpace = true;
    if (unknown_84_2_ && alloc != NULL)
    {
        void* stagingSpace = alloc->Allocate(length);
        if (stagingSpace != NULL)
        {
            memcpy(stagingSpace, data, length);
            data = stagingSpace;
            allocStagingSpace = false;
        }
    }
    StageMemoryToVRAM(VRAMSubregion_TexturePalette, data, offset, length, false, allocStagingSpace);
    paletteVRAMOffset_ = offset;
}

void SPRSprite::CloneTo(SPRSprite* out)
{
    out->numTextures_ = numTextures_;
    out->textures_ = textures_;
    out->animations_ = animations_;
    out->paletteVRAMOffset_ = paletteVRAMOffset_;
    out->animationNames_.count = animationNames_.count;
    out->animationNames_.array = animationNames_.array;
    out->textureFormat_ = textureFormat_;
    out->unknown_84_1_ = unknown_84_1_;
    out->initialized_ = initialized_;
}

void SPRSprite::Names::Reset()
{
    count = 0;
    array = NULL;
}

void SPRSprite::Names::Allocate(int num, SafeAllocator* alloc)
{
    count = num;
    array = (AnimationName*)alloc->Allocate(count * sizeof(AnimationName));
}

int SPRSprite::Names::Find(const char* str)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(str, array[i]) == 0)
            return i;
    }
    return 0;
}