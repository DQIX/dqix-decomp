#include "System/BGBases.h"
#include "System/Graphics.h"

// from gbatek:
// BG Mode
// Engine A BG Mode (DISPCNT LSBs) (0-6, 7=Reserved)
// 
//   Mode  BG0      BG1      BG2      BG3
//   0     Text/3D  Text     Text     Text
//   1     Text/3D  Text     Text     Affine
//   2     Text/3D  Text     Affine   Affine
//   3     Text/3D  Text     Text     Extended
//   4     Text/3D  Text     Affine   Extended
//   5     Text/3D  Text     Extended Extended
//   6     3D       -        Large    -

unsigned int GetMainBG0ScreenBase()
{
    int bg0 = BG0CNT;
    unsigned int dispcnt = DISPCNT;
    dispcnt = (dispcnt & DISPCNT_MASK_SCREEN_BASE_64K) >> 27;
    bg0 = (bg0 & BGCNT_MASK_SCREEN_BASE_2K) >> 8;
    return (dispcnt * 64 * 1024) + 0x06000000 + (bg0 * 2 * 1024);
}

unsigned int GetSubBG0ScreenBase()
{
    return ((BG0CNTSUB & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (2 * 1024) + 0x06200000;
}

unsigned int GetMainBG1ScreenBase()
{
    int bg1 = BG1CNT;
    unsigned int dispcnt = DISPCNT;
    dispcnt = (dispcnt & DISPCNT_MASK_SCREEN_BASE_64K) >> 27;
    bg1 = (bg1 & BGCNT_MASK_SCREEN_BASE_2K) >> 8;
    return (dispcnt * 64 * 1024) + 0x06000000 + (bg1 * 2 * 1024);
}

unsigned int GetSubBG1ScreenBase()
{
    return ((BG1CNTSUB & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (2 * 1024) + 0x06200000;
}

unsigned int GetMainBG2ScreenBase()
{
    unsigned int mode = DISPCNT & 7;
    unsigned int bg2Control = BG2CNT;
    
    unsigned int offsetFromDispcnt = ((DISPCNT & DISPCNT_MASK_SCREEN_BASE_64K) >> 27) << 16;
    int bg2OffsetBits = (bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8;
    
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        return offsetFromDispcnt + 0x06000000 + bg2OffsetBits * (2 * 1024);
    case 5:
        if (bg2Control & BGCNT_MASK_PALETTE_256)
            return ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (16 * 1024) + 0x06000000;
        else
            return offsetFromDispcnt + 0x06000000 + ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (2 * 1024);
    case 6:
        return 0x06000000;
    }
    return 0;
}

unsigned int GetSubBG2ScreenBase()
{
    unsigned int mode = DISPCNTSUB & 7;
    unsigned int bg2Control = BG2CNTSUB;
    
    int bg2OffsetBits = (bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8;
    
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        return 0x06200000 + bg2OffsetBits * (2 * 1024);
    case 5:
        if (bg2Control & BGCNT_MASK_PALETTE_256)
            return ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (16 * 1024) + 0x06200000;
        else
            return 0x06200000 + ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (2 * 1024);
    case 6:
        return 0;
    }
    return 0;
}

unsigned int GetMainBG3ScreenBase()
{
    unsigned int mode = DISPCNT & 7;
    unsigned int bg2Control = BG3CNT;
    
    unsigned int offsetFromDispcnt = ((DISPCNT & DISPCNT_MASK_SCREEN_BASE_64K) >> 27) << 16;
    int bg2OffsetBits = (bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8;
    
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
        return offsetFromDispcnt + 0x06000000 + bg2OffsetBits * (2 * 1024);
    case 3:
    case 4:
    case 5:
        if (bg2Control & BGCNT_MASK_PALETTE_256)
            return ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (16 * 1024) + 0x06000000;
        else
            return offsetFromDispcnt + 0x06000000 + ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (2 * 1024);
    case 6:
        return 0;
    }
    return 0;
}

unsigned int GetSubBG3ScreenBase()
{
    unsigned int mode = DISPCNTSUB & 7;
    unsigned int bg2Control = BG3CNTSUB;
    
    int bg2OffsetBits = (bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8;
    
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
        return 0x06200000 + bg2OffsetBits * (2 * 1024);
    case 3:
    case 4:
    case 5:
        if (bg2Control & BGCNT_MASK_PALETTE_256)
            return ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (16 * 1024) + 0x06200000;
        else
            return 0x06200000 + ((bg2Control & BGCNT_MASK_SCREEN_BASE_2K) >> 8) * (2 * 1024);
    case 6:
        return 0;
    }
    return 0;
}

unsigned int GetMainBG0CharacterBase()
{
    int bg0 = BG0CNT;
    unsigned int dispcnt = DISPCNT;

    dispcnt = (dispcnt & DISPCNT_MASK_CHARACTER_BASE_64K) >> 24;
    bg0 = (bg0 & BGCNT_MASK_CHARACTER_BASE_16K) >> 2;

    return (dispcnt * 64 * 1024) + 0x06000000 + (bg0 * 16 * 1024);
}

unsigned int GetSubBG0CharacterBase()
{
    return ((BG0CNTSUB & BGCNT_MASK_CHARACTER_BASE_16K) >> 2) * 16 * 1024
        + 0x06200000;
}

unsigned int GetMainBG1CharacterBase()
{
    int bg1 = BG1CNT;
    unsigned int dispcnt = DISPCNT;

    dispcnt = (dispcnt & DISPCNT_MASK_CHARACTER_BASE_64K) >> 24;
    bg1 = (bg1 & BGCNT_MASK_CHARACTER_BASE_16K) >> 2;

    return (dispcnt * 64 * 1024) + 0x06000000 + (bg1 * 16 * 1024);
}

unsigned int GetSubBG1CharacterBase()
{
    return ((BG1CNTSUB & BGCNT_MASK_CHARACTER_BASE_16K) >> 2) * 16 * 1024
        + 0x06200000;
}

unsigned int GetMainBG2CharacterBase()
{
    int mode = DISPCNT & 7;
    unsigned int bg2 = BG2CNT;
    if (mode < 5 || !(bg2 & BGCNT_MASK_PALETTE_256))
    {
        return ((DISPCNT & DISPCNT_MASK_CHARACTER_BASE_64K) >> 24) * (64 * 1024)
            + 0x06000000
            + ((bg2 & BGCNT_MASK_CHARACTER_BASE_16K) >> 2) * (16 * 1024);
    }
    else
        return 0;
}

unsigned int GetSubBG2CharacterBase()
{
    int mode = DISPCNTSUB & 7;
    unsigned int bg2 = BG2CNTSUB;
    if (mode < 5 || !(bg2 & BGCNT_MASK_PALETTE_256))
    {
        return 0x06200000
            + ((bg2 & BGCNT_MASK_CHARACTER_BASE_16K) >> 2) * (16 * 1024);
    }
    else
        return 0;
}

unsigned int GetMainBG3CharacterBase()
{
    int mode = DISPCNT & 7;
    unsigned int bg3 = BG3CNT;
    if (mode < 3 || (mode < 6 && !(bg3 & BGCNT_MASK_PALETTE_256)))
    {
        return ((DISPCNT & DISPCNT_MASK_CHARACTER_BASE_64K) >> 24) * (64 * 1024)
            + 0x06000000
            + ((bg3 & BGCNT_MASK_CHARACTER_BASE_16K) >> 2) * (16 * 1024);
    }
    else
        return 0;
}

unsigned int GetSubBG3CharacterBase()
{
    int mode = DISPCNTSUB & 7;
    unsigned int bg3 = BG3CNTSUB;
    if (mode < 3 || (mode < 6 && !(bg3 & BGCNT_MASK_PALETTE_256)))
    {
        return 0x06200000
            + ((bg3 & BGCNT_MASK_CHARACTER_BASE_16K) >> 2) * (16 * 1024);
    }
    else
        return 0;
}