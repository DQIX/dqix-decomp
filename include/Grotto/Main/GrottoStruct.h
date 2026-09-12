#pragma once

#include "TreasureMapMetadata.h"

struct DetailedTreasureMapData;

// Could probably do with a better name. This is a persistent struct
// holding data about all grottos, as opposed to the ActiveGrottoStruct
// which only holds data about a single grotto while you're inside it.
struct GrottoStruct
{
    unsigned char unknown_0[5];
    // still a bit unsure of these, though seems correct.
    unsigned char activeEnviron;
    unsigned char activeStartingMonsterRank;
    unsigned char activeMapLevel;

    unsigned char unknown_8, unknown_9;
    char unk_a[2];

    unsigned short entranceZoneId;
    int entranceX, entranceY, entranceZ; // centre of the grotto entrance model
    char activeMapImageName[16]; // e.g. tmap_005
#if defined(usa)
    char activeMapNameNoLevel[64]; // e.g. Granite Tunnel of Woe
#elif defined(jpn)                 // (this is the string that appears
    char activeMapNameNoLevel[32]; // on the top screen in overworld)
#endif

    TreasureMapMetadata activeMapData;
    char unk_88[0x7c]; // not sure about this part for jpn version
    unsigned char numMaps;
    char padding[1];
    TreasureMapMetadata maps[99];

    void LoadActiveMetadataFromDetailed(DetailedTreasureMapData* detail);
};

class GameState;