#pragma once

#include "TreasureMapMetadata.h"

struct GrottoStruct
{
    unsigned char unknown1[7];
    unsigned char activeMapLevel;
    unsigned char unknown2[0x64];
    TreasureMapMetadata activeMapData;
    char unknown3[0x7c];
    unsigned char numMaps;
    char padding[1];
    TreasureMapMetadata maps[99];
};

struct BattleStruct;

// Given the signature of this, it seems likely that the battle struct and
// grotto struct are actually part of a larger object with the battle struct
// occupying the first bytes.
GrottoStruct* GetGrottoStruct(BattleStruct* battle);