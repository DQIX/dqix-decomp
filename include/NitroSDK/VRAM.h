#pragma once

#include "std_library_functions.h"

#define VRAM_BANK_A 1
#define VRAM_BANK_B 2
#define VRAM_BANK_C 4
#define VRAM_BANK_D 8
#define VRAM_BANK_E 0x10
#define VRAM_BANK_F 0x20
#define VRAM_BANK_G 0x40
#define VRAM_BANK_H 0x80
#define VRAM_BANK_I 0x100

#if defined(jpn)
#define data_02111224 data_02110ec4
#endif

struct VRAMBankUsages
{
    unsigned short lcdcMappedBanks_; // 24
    unsigned short mainBGBanks_; // 26
    unsigned short mainOBJBanks_; // 28
    unsigned short arm7WorkRAMBanks_; // 2a
    unsigned short textureImageBanks_; // 2c
    unsigned short texturePaletteBanks_; // 2e
    unsigned short clearTextureBanks_; // 30
    unsigned short mainBGExtPaletteBanks_; // 32
    unsigned short mainObjExtPaletteBanks_; // 34
    unsigned short subBGBanks_; // 36
    unsigned short subOBJBanks_; // 38
    unsigned short subBGExtPaletteBanks_; // 3a
    unsigned short subObjExtPaletteBanks_; // 3c
} extern data_02111224;

extern "C"
{
    void InitializeVRAM();
    void InternalMapVRAMBanksToLCDC(int banks);
    // accepts either a subset of {A, B, C, D} or a subset of {E, F, G}
    void MapVRAMBanksToMainBG(int banks);
    // accepts either a subset of {A, B} or a subset of {E, F, G}
    void MapVRAMBanksToMainObj(int banks);
    // accepts either E or a subset of {F, G}.
    // only 32k of E can be used.
    void MapVRAMBanksToMainBGExtendedPalette(int banks);
    // accepts either F or G.
    // only 8k can be used.
    void MapVRAMBanksToMainObjExtendedPalette(int banks);
    // accepts a subset of {A, B, C, D}
    void MapVRAMBanksToTextureImage(int banks);
    // accepts a subset of {E, F, G} except for {E, G}
    void MapVRAMBanksToTexturePalette(int banks);
    // acceptable options are A, B, A+B, C, D, C+D.
    // GBATEK seems very cryptic here, but as far as I can tell this lets
    // you clear the 'color buffer' and 'depth buffer' (to the extent that
    // these exist in the NDS for 3D graphics) to a texture/bitmap instead
    // of a solid color/value.
    // If only one bank is provided, it is mapped to offset 3, and if two are
    // provided they are mapped to 2 and 3. From looking at melonDS source
    // code offset 2 is for color and 3 for depth. Maybe the idea is that if
    // offset 2 isn't mapped then it defaults to black/transparent?
    void MapVRAMBanksToClearTextures(int banks);
    // accepts a subset of {C, D}
    void MapVRAMBanksToArm7WorkRAM(int banks);
    // accepts any subset
    void MapVRAMBanksToLCDC(int banks);
    // accepts {C}, {H} or {H, I}
    void MapVRAMBanksToSubBG(int banks);
    // accepts {D} or {I}
    void MapVRAMBanksToSubObj(int banks);
    // accepts {H}
    void MapVRAMBanksToSubBGExtendedPalette(int banks);
    // accepts {I}
    void MapVRAMBanksToSubObjExtendedPalette(int banks);

    // Reads the bank mask at the specified address, resets those
    // banks to LCDC mode, sets the mask to 0 and returns the prior
    // value. In practice it's used with members of data_02111224
    // to 'release' all VRAM banks used for a specific purpose
    unsigned short ResetAssignedVRAMBanksByUsage(unsigned short* usage);

    // Release/free all VRAM banks currently used for a specific purpose.
    // As far as I know, the data within the bank is not cleared/changed.
    // Returns a mask of all banks affected.
    // These functions might be supposed to return (unsigned) int instead
    unsigned short ReleaseMainBGVRAMBanks();
    //unsigned short ReleaseMainObjVRAMBanks(); // this one doesn't exist!
    unsigned short ReleaseMainBGExtendedPaletteVRAMBanks();
    unsigned short ReleaseMainObjExtendedPaletteVRAMBanks();
    unsigned short ReleaseTextureImageVRAMBanks();
    unsigned short ReleaseTexturePaletteVRAMBanks();
    unsigned short ReleaseClearTextureVRAMBanks();
    unsigned short ReleaseSubBGVRAMBanks();
    unsigned short ReleaseSubObjVRAMBanks();
    unsigned short ReleaseSubBGExtendedPaletteVRAMBanks();
    unsigned short ReleaseSubObjExtendedPaletteVRAMBanks();

    // Disable (set control = 0) VRAM banks specified by the mask,
    // set the mask to zero and return the old mask. It also calls 
    // func_020c9a88 which handles some data that might be shared with the arm7?
    // it uses the same mechanism of acquiring a uint16 owner id as seen in
    // the gamecard bus ownership stuff
    unsigned int DisableAssignedVRAMBanksByUsage(unsigned short* usage);

    // Disable all VRAM banks currently used for a specific purpose.
    // As far as I know, the data within the bank is not cleared/changed.
    // Returns a mask of all banks affected
    unsigned int DisableMainBGVRAMBanks();
    unsigned int DisableMainObjVRAMBanks();
    unsigned int DisableMainBGExtendedPaletteVRAMBanks();
    unsigned int DisableMainObjExtendedPaletteVRAMBanks();
    unsigned int DisableTextureImageVRAMBanks();
    unsigned int DisableTexturePaletteVRAMBanks();
    unsigned int DisableClearTextureVRAMBanks();
    unsigned int DisableArm7WorkVRAMBanks();
    unsigned int DisableLCDCMappedVRAMBanks();
    unsigned int DisableSubBGVRAMBanks();
    unsigned int DisableSubObjVRAMBanks();
    unsigned int DisableSubBGExtendedPaletteVRAMBanks();
    unsigned int DisableSubObjExtendedPaletteVRAMBanks();

    unsigned short GetMainBGVRAMBanks();
    unsigned short GetMainObjVRAMBanks();
    unsigned short GetMainBGExtendedPaletteVRAMBanks();
    unsigned short GetMainObjExtendedPaletteVRAMBanks();
    unsigned short GetTextureImageVRAMBanks();
    unsigned short GetTexturePaletteVRAMBanks();
    unsigned short GetSubBGVRAMBanks();
    unsigned short GetSubObjVRAMBanks();
    unsigned short GetSubBGExtendedPaletteVRAMBanks();
    unsigned short GetSubObjExtendedPaletteVRAMBanks();

    unsigned int GetTotalComponentVRAMSize(int banks);

    unsigned int GetMainBGAssignedVRAMSize();
    unsigned int GetTextureImageAssignedVRAMSize();
    unsigned int GetTexturePaletteAssignedVRAMSize();
}