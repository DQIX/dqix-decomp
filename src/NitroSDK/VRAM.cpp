#include "System/VRAM.h"
#include "System/Graphics.h"

#pragma optimize_for_size off

#if defined(jpn)
#define data_02111222 data_02110ec2

#define func_020c9a88 func_020cb554
#endif

extern unsigned short data_02111222;

extern "C"
{
    void func_020c9a88(unsigned short banks, unsigned short owner);
}

void InitializeVRAM()
{
    data_02111224.lcdcMappedBanks_ = 0;
    data_02111224.mainBGBanks_ = 0;
    data_02111224.mainOBJBanks_ = 0;
    data_02111224.arm7WorkRAMBanks_ = 0;
    data_02111224.textureImageBanks_ = 0;
    data_02111224.texturePaletteBanks_ = 0;
    data_02111224.clearTextureBanks_ = 0;
    data_02111224.mainBGExtPaletteBanks_ = 0;
    data_02111224.mainObjExtPaletteBanks_ = 0;
    data_02111224.subBGBanks_ = 0;
    data_02111224.subOBJBanks_ = 0;
    data_02111224.subBGExtPaletteBanks_ = 0;
    data_02111224.subObjExtPaletteBanks_ = 0;

    intptr_t vramAddr = 0x04000240;
    *(int*)vramAddr = 0; // banks A, B, C, D
    *(char*)(vramAddr + 4) = 0; // bank E
    *(char*)(vramAddr + 5) = 0; // bank F
    *(char*)(vramAddr + 6) = 0; // bank G
    *(short*)(vramAddr + 8) = 0; // banks H, I
}

void InternalMapVRAMBanksToLCDC(int banks)
{
    // 0x80 = enabled, MST = 0 (LCDC), offset = 0 (not relevant)
    if (banks & VRAM_BANK_A)
        VRAMCNT_A = 0x80;
    if (banks & VRAM_BANK_B)
        VRAMCNT_B = 0x80;
    if (banks & VRAM_BANK_C)
        VRAMCNT_C = 0x80;
    if (banks & VRAM_BANK_D)
        VRAMCNT_D = 0x80;
    if (banks & VRAM_BANK_E)
        VRAMCNT_E = 0x80;
    if (banks & VRAM_BANK_F)
        VRAMCNT_F = 0x80;
    if (banks & VRAM_BANK_G)
        VRAMCNT_G = 0x80;
    if (banks & VRAM_BANK_H)
        VRAMCNT_H = 0x80;
    if (banks & VRAM_BANK_I)
        VRAMCNT_I = 0x80;
}

extern "C" void MapVRAMBanksToMainBG(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.mainBGBanks_);
    data_02111224.mainBGBanks_ = banks;
    // You can either use any of banks A,B,C,D, or you can use any of E,F,G
    // but you can't mix the two sets.
    // A,B,C,D are all 128k and are mapped consecutively to 06000000, 06020000, ...
    // E is 64k and will be mapped to 06000000 (offset 0)
    // F, G are 16k and are mapped to 06000000 & 06004000 (offsets 0 and 1) if used without E
    // or 06010000 and 06014000 (offsets 2 and 3) if used with E.
    // All of these use MST = 1 (means main BG for banks A-G), for which
    // offsets 0, 1, 2, 3 give flags 0x81, 0x89, 0x91, 0x99 respectively.
    switch (banks)
    {
    case VRAM_BANK_D:
        VRAMCNT_D = 0x81;
        break;
    case VRAM_BANK_C | VRAM_BANK_D:
        VRAMCNT_D = 0x89;
    case VRAM_BANK_C:
        VRAMCNT_C = 0x81;
        break;
    case VRAM_BANK_B | VRAM_BANK_C | VRAM_BANK_D:
        VRAMCNT_D = 0x91;
    case VRAM_BANK_B | VRAM_BANK_C:
        VRAMCNT_C = 0x89;
    case VRAM_BANK_B:
        VRAMCNT_B = 0x81;
        break;
    case VRAM_BANK_A | VRAM_BANK_B | VRAM_BANK_C | VRAM_BANK_D:
        VRAMCNT_D = 0x99;
    case VRAM_BANK_A | VRAM_BANK_B | VRAM_BANK_C:
        VRAMCNT_C = 0x91;
    case VRAM_BANK_A | VRAM_BANK_B:
        VRAMCNT_B = 0x89;
    case VRAM_BANK_A:
        VRAMCNT_A = 0x81;
        break;
    case VRAM_BANK_A | VRAM_BANK_B | VRAM_BANK_D:
        VRAMCNT_A = 0x81;
        VRAMCNT_B = 0x89;
        VRAMCNT_D = 0x91;
        break;
    case VRAM_BANK_A | VRAM_BANK_C | VRAM_BANK_D:
        VRAMCNT_D = 0x91;
    case VRAM_BANK_A | VRAM_BANK_C:
        VRAMCNT_A = 0x81;
        VRAMCNT_C = 0x89;
        break;
    case VRAM_BANK_A | VRAM_BANK_D:
        VRAMCNT_A = 0x81;
        VRAMCNT_D = 0x89;
        break;
    case VRAM_BANK_B | VRAM_BANK_D:
        VRAMCNT_B = 0x81;
        VRAMCNT_D = 0x89;
        break;
    
    
    case VRAM_BANK_E | VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x99;
    case VRAM_BANK_E | VRAM_BANK_F:
        VRAMCNT_F = 0x91;
    case VRAM_BANK_E:
        VRAMCNT_E = 0x81;
        break;
    case VRAM_BANK_E | VRAM_BANK_G:
        VRAMCNT_G = 0x91;
        VRAMCNT_E = 0x81;
        break;
    
    case VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x89;
    case VRAM_BANK_F:
        VRAMCNT_F = 0x81;
        break;
    case VRAM_BANK_G:
        VRAMCNT_G = 0x81;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

extern "C" void MapVRAMBanksToMainObj(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.mainOBJBanks_);
    data_02111224.mainOBJBanks_ = banks;
    // You can either use A and/or B, or you can use any of E,F,G.
    // Offset assignments are like with main BG (see previous function).
    // Here MST = 2 in all cases, so offsets 0,1,2,3 correspond to
    // 0x82, 0x8a, 0x92, 0x9a respectively.
    switch (banks)
    {
    case 0:
        break;
    case VRAM_BANK_A | VRAM_BANK_B:
        VRAMCNT_B = 0x8a;
    case VRAM_BANK_A:
        VRAMCNT_A = 0x82;
        break;
    case VRAM_BANK_B:
        VRAMCNT_B = 0x82;
        break;

    case VRAM_BANK_E | VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x9a;
    case VRAM_BANK_E | VRAM_BANK_F:
        VRAMCNT_F = 0x92;
    case VRAM_BANK_E:
        VRAMCNT_E = 0x82;
        break;
    case VRAM_BANK_E | VRAM_BANK_G:
        VRAMCNT_G = 0x92;
        VRAMCNT_E = 0x82;
        break;
    case VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x8a;
    case VRAM_BANK_F:
        VRAMCNT_F = 0x82;
        break;
    case VRAM_BANK_G:
        VRAMCNT_G = 0x82;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToMainBGExtendedPalette(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.mainBGExtPaletteBanks_);
    data_02111224.mainBGExtPaletteBanks_ = banks;
    // MST = 4 and offset 0 or 1: 0x84 or 0x8c
    switch (banks)
    {
    case VRAM_BANK_E:
        DISPCNT |= DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
        VRAMCNT_E = 0x84;
        break;
    case VRAM_BANK_G:
        DISPCNT |= DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
        VRAMCNT_G = 0x8c;
        break;
    case VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x8c;
    case VRAM_BANK_F:
        VRAMCNT_F = 0x84;
        DISPCNT |= DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
        break;
    
    case 0:
        DISPCNT &= ~DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToMainObjExtendedPalette(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.mainObjExtPaletteBanks_);
    data_02111224.mainObjExtPaletteBanks_ = banks;
    // MST = 5, only offset 0 can work
    switch (banks)
    {
    case VRAM_BANK_F:
        DISPCNT |= DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
        VRAMCNT_F = 0x85;
        break;
    case VRAM_BANK_G:
        DISPCNT |= DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
        VRAMCNT_G = 0x85;
        break;
    case 0:
        DISPCNT &= ~DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToTextureImage(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.textureImageBanks_);
    data_02111224.textureImageBanks_ = banks;
    // MST = 3 for A-D means texture image.
    // Offsets 0, 1, 2, 3 are 0x83, 0x8b, 0x93, 0x9b respectively.
    if (banks == 0)
        DISP3DCNT &= 0xffff & ~((1 << 0) | (1 << 12) | (1 << 13));
    else
    {
        DISP3DCNT = (DISP3DCNT & ~((1 << 12) | (1 << 13))) | (1 << 0);
        switch (banks)
        {
        case VRAM_BANK_A | VRAM_BANK_C:
            VRAMCNT_A = 0x83;
            VRAMCNT_C = 0x8b;
            break;
        case VRAM_BANK_A | VRAM_BANK_D:
            VRAMCNT_A = 0x83;
            VRAMCNT_D = 0x8b;
            break;
        case VRAM_BANK_B | VRAM_BANK_D:
            VRAMCNT_B = 0x83;
            VRAMCNT_D = 0x8b;
            break;
        case VRAM_BANK_A | VRAM_BANK_B | VRAM_BANK_D:
            VRAMCNT_A = 0x83;
            VRAMCNT_B = 0x8b;
            VRAMCNT_D = 0x93;
            break;
        case VRAM_BANK_A | VRAM_BANK_C | VRAM_BANK_D:
            VRAMCNT_A = 0x83;
            VRAMCNT_C = 0x8b;
            VRAMCNT_D = 0x93;
            break;
        case VRAM_BANK_D:
            VRAMCNT_D = 0x83;
            break;
        case VRAM_BANK_C | VRAM_BANK_D:
            VRAMCNT_D = 0x8b;
        case VRAM_BANK_C:
            VRAMCNT_C = 0x83;
            break;
        case VRAM_BANK_B | VRAM_BANK_C | VRAM_BANK_D:
            VRAMCNT_D = 0x93;
        case VRAM_BANK_B | VRAM_BANK_C:
            VRAMCNT_C = 0x8b;
        case VRAM_BANK_B:
            VRAMCNT_B = 0x83;
            break;
        case VRAM_BANK_A | VRAM_BANK_B | VRAM_BANK_C | VRAM_BANK_D:
            VRAMCNT_D = 0x9b;
        case VRAM_BANK_A | VRAM_BANK_B | VRAM_BANK_C:
            VRAMCNT_C = 0x93;
        case VRAM_BANK_A | VRAM_BANK_B:
            VRAMCNT_B = 0x8b;
        case VRAM_BANK_A:
            VRAMCNT_A = 0x83;
        }
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToTexturePalette(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.texturePaletteBanks_);
    data_02111224.texturePaletteBanks_ = banks;
    // MST = 3 for E,F,G means texture palette.
    // Offset 2 for F or G puts it right after E ends
    switch (banks)
    {
    case 0:
        break;
    case VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x8b;
    case VRAM_BANK_F:
        VRAMCNT_F = 0x83;
        break;
    case VRAM_BANK_G:
        VRAMCNT_G = 0x83;
        break;
    case VRAM_BANK_E | VRAM_BANK_F | VRAM_BANK_G:
        VRAMCNT_G = 0x9b;
    case VRAM_BANK_E | VRAM_BANK_F:
        VRAMCNT_F = 0x93;
    case VRAM_BANK_E:
        VRAMCNT_E = 0x83;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToClearTextures(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.clearTextureBanks_);
    data_02111224.clearTextureBanks_ = banks;
    switch (banks)
    {
    case VRAM_BANK_A | VRAM_BANK_B:
        VRAMCNT_A = 0x93;
    case VRAM_BANK_B:
        VRAMCNT_B = 0x9b;
        DISP3DCNT |= DISP3DCNT_USE_CLEAR_TEXTURES;
        break;
    case VRAM_BANK_C | VRAM_BANK_D:
        VRAMCNT_C = 0x93;
    case VRAM_BANK_D:
        VRAMCNT_D = 0x9b;
        DISP3DCNT |= DISP3DCNT_USE_CLEAR_TEXTURES;
        break;
    case 0:
        DISP3DCNT &= ~DISP3DCNT_USE_CLEAR_TEXTURES;
        break;
    case VRAM_BANK_A:
        VRAMCNT_A = 0x9b;
        DISP3DCNT |= DISP3DCNT_USE_CLEAR_TEXTURES;
        break;
    case VRAM_BANK_C:
        VRAMCNT_C = 0x9b;
        DISP3DCNT |= DISP3DCNT_USE_CLEAR_TEXTURES;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

// This match is a war crime but switch(...) just wasn't cooperating here
void MapVRAMBanksToArm7WorkRAM(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.arm7WorkRAMBanks_);
    data_02111224.arm7WorkRAMBanks_ = banks;
    if (banks <= 8)
    {
        if (banks >= 8)
            goto workram_case_D;
        // I do love that this is entirely equivalent to banks != 4
        if (banks > 4 || banks < 0 || banks == 0 || banks != 4)
            goto workram_switch_end;
        goto workram_case_C;
    }
    else
    {
        if (banks != 12)
            goto workram_switch_end;
        // falls through to case C+D
    }

workram_case_C_D:
    VRAMCNT_D = 0x8a;
    VRAMCNT_C = 0x82;
    goto workram_switch_end;
workram_case_C:
    VRAMCNT_C = 0x82;
    goto workram_switch_end;
workram_case_D:
    VRAMCNT_D = 0x82;
    goto workram_switch_end;
workram_switch_end:
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToLCDC(int banks)
{
    data_02111224.lcdcMappedBanks_ |= banks;
    InternalMapVRAMBanksToLCDC(banks);
}

// more war crimes here
void MapVRAMBanksToSubBG(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.subBGBanks_);
    data_02111224.subBGBanks_ = banks;
    // this one uses MST 4 for bank C, but MST 1 for H and I (no offsets)
    if (banks <= 0x80)
    {
        if (banks >= 0x80)
            goto subBG_case_H;

        if (banks > 4 || banks < 0 || banks == 0 || banks != 4)
            goto subBG_switch_end;
        // goes to case C
    }
    else
    {
        if (banks != 0x180)
            goto subBG_switch_end;
        goto subBG_case_H_I;
    }
subBG_case_C:
    VRAMCNT_C = 0x84;
    goto subBG_switch_end;
subBG_case_H_I:
    VRAMCNT_I = 0x81;
subBG_case_H:
    VRAMCNT_H = 0x81;
    goto subBG_switch_end;
subBG_switch_end:
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToSubObj(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.subOBJBanks_);
    data_02111224.subOBJBanks_ = banks;
    // this mode uses MST = 4 for D and MST = 2 for I
    switch (banks)
    {
    case 0:
        break;
    case VRAM_BANK_D:
        VRAMCNT_D = 0x84;
        break;
    case VRAM_BANK_I:
        VRAMCNT_I = 0x82;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToSubBGExtendedPalette(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.subBGExtPaletteBanks_);
    data_02111224.subBGExtPaletteBanks_ = banks;
    switch (banks)
    {
    case VRAM_BANK_H:
        DISPCNTSUB |= DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
        VRAMCNT_H = 0x82;
        break;
    case 0:
        DISPCNTSUB &= ~DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

void MapVRAMBanksToSubObjExtendedPalette(int banks)
{
    data_02111224.lcdcMappedBanks_ = ~banks & (data_02111224.lcdcMappedBanks_ | data_02111224.subObjExtPaletteBanks_);
    data_02111224.subObjExtPaletteBanks_ = banks;
    switch (banks)
    {
    case VRAM_BANK_I:
        DISPCNTSUB |= DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
        VRAMCNT_I = 0x83;
        break;
    case 0:
        DISPCNTSUB &= ~DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
        break;
    }
    InternalMapVRAMBanksToLCDC(data_02111224.lcdcMappedBanks_);
}

unsigned short ResetAssignedVRAMBanksByUsage(unsigned short* usage)
{
    int banks = *usage;
    *usage = 0;
    data_02111224.lcdcMappedBanks_ |= banks;
    InternalMapVRAMBanksToLCDC(banks);
    return banks;
}

unsigned short ReleaseMainBGVRAMBanks()
{
    return ResetAssignedVRAMBanksByUsage(&data_02111224.mainBGBanks_);
}

unsigned short ReleaseMainBGExtendedPaletteVRAMBanks()
{
    DISPCNT &= ~DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
    return ResetAssignedVRAMBanksByUsage(&data_02111224.mainBGExtPaletteBanks_);
}

unsigned short ReleaseMainObjExtendedPaletteVRAMBanks()
{
    DISPCNT &= ~DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
    return ResetAssignedVRAMBanksByUsage(&data_02111224.mainObjExtPaletteBanks_);
}

unsigned short ReleaseTextureImageVRAMBanks()
{
    return ResetAssignedVRAMBanksByUsage(&data_02111224.textureImageBanks_);
}

unsigned short ReleaseTexturePaletteVRAMBanks()
{
    return ResetAssignedVRAMBanksByUsage(&data_02111224.texturePaletteBanks_);
}

unsigned short ReleaseClearTextureVRAMBanks()
{
    return ResetAssignedVRAMBanksByUsage(&data_02111224.clearTextureBanks_);
}

unsigned short ReleaseSubBGVRAMBanks()
{
    return ResetAssignedVRAMBanksByUsage(&data_02111224.subBGBanks_);
}

unsigned short ReleaseSubObjVRAMBanks()
{
    return ResetAssignedVRAMBanksByUsage(&data_02111224.subOBJBanks_);
}

unsigned short ReleaseSubBGExtendedPaletteVRAMBanks()
{
    DISPCNTSUB &= ~DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
    return ResetAssignedVRAMBanksByUsage(&data_02111224.subBGExtPaletteBanks_);
}

unsigned short ReleaseSubObjExtendedPaletteVRAMBanks()
{
    DISPCNTSUB &= ~DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
    return ResetAssignedVRAMBanksByUsage(&data_02111224.subObjExtPaletteBanks_);
}

unsigned int DisableAssignedVRAMBanksByUsage(unsigned short* usage)
{
    unsigned int banks = *usage;
    *usage = 0;
    if (banks & VRAM_BANK_A)
        VRAMCNT_A = 0;
    if (banks & VRAM_BANK_B)
        VRAMCNT_B = 0;
    if (banks & VRAM_BANK_C)
        VRAMCNT_C = 0;
    if (banks & VRAM_BANK_D)
        VRAMCNT_D = 0;
    if (banks & VRAM_BANK_E)
        VRAMCNT_E = 0;
    if (banks & VRAM_BANK_F)
        VRAMCNT_F = 0;
    if (banks & VRAM_BANK_G)
        VRAMCNT_G = 0;
    if (banks & VRAM_BANK_H)
        VRAMCNT_H = 0;
    if (banks & VRAM_BANK_I)
        VRAMCNT_I = 0;
    func_020c9a88(banks, data_02111222);
    return banks;
}

unsigned int DisableMainBGVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.mainBGBanks_);
}

unsigned int DisableMainObjVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.mainOBJBanks_);
}

unsigned int DisableMainBGExtendedPaletteVRAMBanks()
{
    DISPCNT &= ~DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
    return DisableAssignedVRAMBanksByUsage(&data_02111224.mainBGExtPaletteBanks_);
}

unsigned int DisableMainObjExtendedPaletteVRAMBanks()
{
    DISPCNT &= ~DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
    return DisableAssignedVRAMBanksByUsage(&data_02111224.mainObjExtPaletteBanks_);
}

unsigned int DisableTextureImageVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.textureImageBanks_);
}

unsigned int DisableTexturePaletteVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.texturePaletteBanks_);
}

unsigned int DisableClearTextureVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.clearTextureBanks_);
}

unsigned int DisableArm7WorkVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.arm7WorkRAMBanks_);
}

unsigned int DisableLCDCMappedVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.lcdcMappedBanks_);
}

unsigned int DisableSubBGVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.subBGBanks_);
}

unsigned int DisableSubObjVRAMBanks()
{
    return DisableAssignedVRAMBanksByUsage(&data_02111224.subOBJBanks_);
}

unsigned int DisableSubBGExtendedPaletteVRAMBanks()
{
    DISPCNTSUB &= ~DISPCNT_ENABLE_BG_EXTENDED_PALETTE;
    return DisableAssignedVRAMBanksByUsage(&data_02111224.subBGExtPaletteBanks_);
}

unsigned int DisableSubObjExtendedPaletteVRAMBanks()
{
    DISPCNTSUB &= ~DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE;
    return DisableAssignedVRAMBanksByUsage(&data_02111224.subObjExtPaletteBanks_);
}

unsigned short GetMainBGVRAMBanks() { return data_02111224.mainBGBanks_; }
unsigned short GetMainObjVRAMBanks() { return data_02111224.mainOBJBanks_; }
unsigned short GetMainBGExtendedPaletteVRAMBanks() { return data_02111224.mainBGExtPaletteBanks_; }
unsigned short GetMainObjExtendedPaletteVRAMBanks() { return data_02111224.mainObjExtPaletteBanks_; }
unsigned short GetTextureImageVRAMBanks(){ return data_02111224.textureImageBanks_; }
unsigned short GetTexturePaletteVRAMBanks() { return data_02111224.texturePaletteBanks_; }
unsigned short GetSubBGVRAMBanks() { return data_02111224.subBGBanks_; }
unsigned short GetSubObjVRAMBanks() { return data_02111224.subOBJBanks_; }
unsigned short GetSubBGExtendedPaletteVRAMBanks() { return data_02111224.subBGExtPaletteBanks_; }
unsigned short GetSubObjExtendedPaletteVRAMBanks() { return data_02111224.subObjExtPaletteBanks_; }

unsigned int GetTotalComponentVRAMSize(int banks)
{
    unsigned int total = 0;
    if (banks & VRAM_BANK_A)
        total += 128 * 1024;
    if (banks & VRAM_BANK_B)
        total += 128 * 1024;
    if (banks & VRAM_BANK_C)
        total += 128 * 1024;
    if (banks & VRAM_BANK_D)
        total += 128 * 1024;
    if (banks & VRAM_BANK_E)
        total += 64 * 1024;
    if (banks & VRAM_BANK_F)
        total += 16 * 1024;
    if (banks & VRAM_BANK_G)
        total += 16 * 1024;
    if (banks & VRAM_BANK_H)
        total += 32 * 1024;
    if (banks & VRAM_BANK_I)
        total += 16 * 1024;
    return total;
}

unsigned int GetMainBGAssignedVRAMSize() { return GetTotalComponentVRAMSize(data_02111224.mainBGBanks_); }
unsigned int GetTextureImageAssignedVRAMSize() { return GetTotalComponentVRAMSize(data_02111224.textureImageBanks_); }
unsigned int GetTexturePaletteAssignedVRAMSize() { return GetTotalComponentVRAMSize(data_02111224.texturePaletteBanks_); }