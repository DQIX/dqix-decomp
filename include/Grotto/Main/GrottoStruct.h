#pragma once

#include "TreasureMapMetadata.h"

// Could probably do with a better name. This is a persistent struct
// holding data about all grottos, as opposed to the ActiveGrottoStruct
// which only holds data about a single grotto while you're inside it.
struct GrottoStruct
{
    unsigned char unknown1[5];
    // still a bit unsure of these, though seems correct.
    unsigned char activeEnviron;
    unsigned char activeStartingMonsterRank;
    unsigned char activeMapLevel;

#ifdef jpn
    unsigned char unknown2[0x44];
#else
    unsigned char unknown2[0x64];
#endif
    TreasureMapMetadata activeMapData;
    char unknown3[0x7c]; // not sure about this part for jpn version
    unsigned char numMaps;
    char padding[1];
    TreasureMapMetadata maps[99];
};

struct BattleStruct;

// Given the signature of this, it seems likely that the battle struct and
// grotto struct are actually part of a larger object with the battle struct
// occupying the first bytes.
GrottoStruct* GetGrottoStruct(BattleStruct* battle);