#include "Graphics/TextureArena.h"

void TextureArena::Initialize(unsigned int imageSpace, unsigned int paletteSpace)
{
    SaveTextureImageAllocationState(textureImageStateBase_);
    COPY_ARRAY(textureImageStateCurrent_, textureImageStateBase_);

    SaveTexturePaletteAllocationState(texturePaletteStateBase_);
    COPY_ARRAY(texturePaletteStateCurrent_, texturePaletteStateBase_);

    textureImageHandle_ = 0;

    int remainingImageSpace = imageSpace;
    while (remainingImageSpace > 0)
    {
        if (remainingImageSpace > 0x20000)
        {
            textureImageHandle_ = FrameAllocateTextureImageVRAM(0x20000, false, 0);
            remainingImageSpace -= 0x20000;
        }
        else
        {
            textureImageHandle_ = FrameAllocateTextureImageVRAM(remainingImageSpace, false, 0);
            remainingImageSpace = 0;
        }
    }

    if (imageSpace != 0 && textureImageHandle_ == 0)
        imageSpace = 0;

    texturePaletteHandle_ = 0;
    if (paletteSpace != 0)
        texturePaletteHandle_ = FrameAllocateTexturePaletteVRAM(paletteSpace, false, 0);

    if (paletteSpace != 0 && texturePaletteHandle_ == 0)
        paletteSpace = 0;

    textureImageAllocatedSize_ = imageSpace;
    texturePaletteAllocatedSize_ = paletteSpace;
}

void TextureArena::Reset()
{
    COPY_ARRAY(textureImageStateCurrent_, textureImageStateBase_);
    COPY_ARRAY(texturePaletteStateCurrent_, texturePaletteStateBase_);
}

void TextureArena::Restore() const
{
    RestoreTextureImageAllocationState(textureImageStateCurrent_);
    RestoreTexturePaletteAllocationState(texturePaletteStateCurrent_);
}

void TextureArena::Save()
{
    SaveTextureImageAllocationState(textureImageStateCurrent_);
    SaveTexturePaletteAllocationState(texturePaletteStateCurrent_);
}

void TextureArena::CreateDescendant(TextureArena* to)
{
    COPY_ARRAY(to->textureImageStateBase_, textureImageStateCurrent_);
    COPY_ARRAY(to->textureImageStateCurrent_, textureImageStateCurrent_);

    to->textureImageAllocatedSize_ = textureImageAllocatedSize_;
    to->textureImageHandle_ = textureImageHandle_;

    COPY_ARRAY(to->texturePaletteStateBase_, texturePaletteStateCurrent_);
    COPY_ARRAY(to->texturePaletteStateCurrent_, texturePaletteStateCurrent_);

    to->texturePaletteAllocatedSize_ = texturePaletteAllocatedSize_;
    to->texturePaletteHandle_ = texturePaletteHandle_;
}