#pragma once

extern "C"
{
    void LoadToMainBGStandardPalette(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBGStandardPalette(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainObjStandardPalette(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubObjStandardPalette(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainOAM(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubOAM(const void* data, unsigned int offset, unsigned int length);

    void LoadToMainObjVRAM(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubObjVRAM(const void* data, unsigned int offset, unsigned int length);

    void LoadToMainBG0ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG0ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainBG1ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG1ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainBG2ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG2ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainBG3ScreenData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG3ScreenData(const void* data, unsigned int offset, unsigned int length);

    void LoadToMainBG0CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG0CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainBG1CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG1CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainBG2CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG2CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToMainBG3CharacterData(const void* data, unsigned int offset, unsigned int length);
    void LoadToSubBG3CharacterData(const void* data, unsigned int offset, unsigned int length);

    void MemoryMapMainBGExtendedPalette();
    void LoadToMainBGExtendedPalette(const void* data, unsigned int offset, unsigned int length);
    void MemoryUnmapMainBGExtendedPalette();

    void MemoryMapMainObjExtendedPalette();
    void LoadToMainObjExtendedPalette(const void* data, unsigned int offset, unsigned int length);
    void MemoryUnmapMainObjExtendedPalette();

    void MemoryMapSubBGExtendedPalette();
    void LoadToSubBGExtendedPalette(const void* data, unsigned int offset, unsigned int length);
    void MemoryUnmapSubBGExtendedPalette();

    void MemoryMapSubObjExtendedPalette();
    void LoadToSubObjExtendedPalette(const void* data, unsigned int offset, unsigned int length);
    void MemoryUnmapSubObjExtendedPalette();

    void MemoryMapTextureImage();
    void LoadToTextureImage(const void* data, unsigned int offset, unsigned int length);
    void MemoryUnmapTextureImage();

    void MemoryMapTexturePalette();
    void LoadToTexturePalette(const void* data, unsigned int offset, unsigned int length);
    void MemoryUnmapTexturePalette();

    void MemoryMapClearTexture();
    void LoadClearImage(const void* data, unsigned int length);
    void LoadClearDepthBuffer(const void* data, unsigned int length);
    void MemoryUnmapClearTexture();
}