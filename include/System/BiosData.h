#pragma once

#include "ProcessorContext.h"

// All of this comes from the documentation at:
// https://problemkaputt.de/gbatek.htm

// Exists at 0x027FFE00
struct CartridgeHeader
{
    char gameTitle[12];
    char gameCode[4];
    char makerCode[2];
    unsigned char unitCode;
    unsigned char encryptionSeedSelect;
    unsigned char cartridgeCapacity;
    char reserved_15[7];
    char reserved_1C;
    unsigned char ndsRegion;
    unsigned char romVersion;
    unsigned char autoStart;
    unsigned int arm9RomOffset;
    unsigned int arm9entryAddress;
    unsigned int arm9RamAddress;
    unsigned int arm9RamSize;
    unsigned int arm7RomOffset;
    unsigned int arm7EntryAddress;
    unsigned int arm7RamAddress;
    unsigned int arm7RamSize;

    unsigned int fileNameTableOffset;
    unsigned int fileNameTableSize;
    unsigned int fileAllocTableOffset;
    unsigned int fileAllocTableSize;

    struct OverlayTableValues
    {
        unsigned int arm9TableStart;
        unsigned int arm9TableSize;
        unsigned int arm7TableStart;
        unsigned int arm7TableSize;
    } overlayTableValues;

    unsigned int gamecardBusControlNormalSettings;
    unsigned int gamecardBusControlKEY1Settings;

    char other_stuff_68[0x108];
};

#ifdef jpn
#define data_020f22cc data_020f2438
#endif

extern CartridgeHeader* data_020f22cc;

// Exists at 0x027FFC40
struct BootIndicator
{
    unsigned short bootMode; // 1 = regular, 2 = download play
    char unknown_unimportant_2[0x27FFE00 - 0x27FFC42];
    CartridgeHeader cartHeader;
};

#define BIOS_ADDR_BOOT_INDICATOR 0x027FFC40
#define BIOS_ADDR_CARTRIDGE_HEADER 0x027FFE00
#define BIOS_ADDR_OVERLAY_TABLE_VALUES 0x027FFE50 

inline bool IsDownloadPlay()
{
    BootIndicator* data = (BootIndicator*)BIOS_ADDR_BOOT_INDICATOR;
    return data->bootMode == 2;
}