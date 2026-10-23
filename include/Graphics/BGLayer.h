#pragma once

#include "NitroSDK/LoadToVRAM.h"
#include "Memory/SafeAllocator.h"
#include <std_library_functions.h>

VRAMLoadProc GetCharVRAMLoadProc(int engine, int bgIndex);
VRAMLoadProc GetScreenVRAMLoadProc(int engine, int bgIndex);

// sizeof == 0x20
struct BGLayer
{
    struct CharacterData
    {
        unsigned int vramOffset;
        short numTileEntries;
        // these look like width & height but this isn't relevant
        // for character data! Is it just useless?
        short unknown_6;
        short unknown_8;
        bool use8bpp;
        char unknown_b;

        void Reset();
        int LoadAndStageFromBNCG(VRAMLoadProc proc, const void* data, SafeAllocator*);
        void LoadMetadataFromBNCG(const void* data);
        void StageBNCG(VRAMLoadProc proc, const void* data);
        void StageBNCG(int engine, int bgIndex, const void* data);
        unsigned int GetEndVRAMOffset() const;
        unsigned int GetVRAMSize() const;

    } charData_;

    struct PaletteData
    {
        bool useExtPalette;
        char pad_1[1];
        // bitfield, palette #i is only copied to vram if bit #i is set.
        // if 0xffff all go, but otherwise for some reason this is cast
        // to unsigned char before use (so can only copy first 8 palettes)
        unsigned short palettesInUse;

        int LoadAndStageFromBNCL(VRAMLoadProc proc, const void* data, SafeAllocator*);
        void LoadMetadataFromBNCL(const void* data);
        void StageBNCL(VRAMLoadProc proc, const void* data);
        void StageBNCL(int engine, const void* data);

    } paletteData_;

    struct Sprite
    {
        short width;
        short height;
        char unknown_4;
        char unknown_5;
        short unknown_6;
        int bufferSize;
        unsigned short* indexBuffer;

        void Reset();
        void MaybeDestroy();
        int LoadFromBNSC(VRAMLoadProc unusedProc, const void* data, SafeAllocator* alloc);
        void GetLengthAndClearCache(const void* data);
        void StageBNSC(VRAMLoadProc proc, const void* data);
        void StageBNSC(int engine, int bgIndex, const void* data);

        void SwitchPalette(unsigned short paletteIndex);
        void SwitchPaletteInRectangle(unsigned short paletteIndex, short x, short y, short w, short h);

    } *sprites_;

    unsigned short* indexBuffer_;
    int unknown_18_;
    unsigned char engine_ : 4; // 0 = main, 1 = sub
    unsigned char bgIndex_ : 4; // 0 to 3
    unsigned char numSprites_;
    unsigned char spriteCapacity_;
    unsigned char unknown_1f_;

    Sprite* GetScreenObject(unsigned int idx);
    void AllocateScreenObjects(unsigned int count, SafeAllocator* alloc);
    void Reset();
    void MaybeDestroy();
    void PrepareIndexBuffer(void* buffer);
    void CleanIndexBufferCache(const void* buffer);
    void UploadIndexBuffer(const void* buffer);
    void CleanCacheAndUploadIndexBuffer(const void* buffer);
    void ConfigureTextMode(int sizeIndex);
    void AllocateIndexBuffer(SafeAllocator* alloc);

    int ProcessGraphicsFile(const void* fileData, SafeAllocator* alloc);
    void ProcessGraphicsFileMetadata(const void* fileData);
    void StageGraphicsFileBuffer(const void* fileData);
    // written slightly differently to the normal version but functions
    // identically
    void StageGraphicsFileBuffer2(const void* fileData);

    void SetPriority(int);
    void SetOffset(int x, int y);

    void BlitSpriteToBuffer(unsigned short* destIdxBuffer, const Sprite* obj, 
        short sourceTileX, short sourceTileY,
        short destTileX, short destTileY,
        short width, short height, unsigned short palette);
    
    void BlitSprite(const Sprite* obj, short sourceTileX, short sourceTileY,
        short destTileX, short destTileY,
        short width, short height, unsigned short palette);

    void BlitSpriteByIndex(unsigned int objIdx, short sourceTileX, short sourceTileY,
        short destTileX, short destTileY,
        short width, short height, unsigned short palette);

    void BlitSprite(const Sprite* obj, short destTileX, short destTileY, unsigned short palette);
    void BlitSpriteByIndex(unsigned int objIdx, short destTileX, short destTileY, unsigned short palette);

    void SetPaletteInRegion(short x, short y, short width, short height, unsigned short palette);

    void BlitSpriteRepeated(const Sprite* obj, short width, short height,
        short destTileX, short destTileY, short countX, short countY, unsigned short palette);
    void BlitSpriteByIndexRepeated(unsigned int objIdx, short width, short height,
        short destTileX, short destTileY, short countX, short countY, unsigned short palette);

    // Function looks like it handles extended palette setting, but it
    // almost certainly doesn't work correctly
    void UploadPaletteData(int whichPalette, int indexInPalette, const void* data, unsigned int dataLength);

    void SetTileInRectangle(unsigned short idx, short x, short y, short w, short h, unsigned short palette);

    inline int GetEngine() const { return engine_; }
    inline int GetBGIndex() const { return bgIndex_; }
};