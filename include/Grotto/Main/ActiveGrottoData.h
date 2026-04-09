#pragma once

#include "GrottoStruct.h"
#include "FloorMapGenerator.h"

// Very WIP. I think in the USA version, the size of this should be
// 632 (0x278), but I don't know what the last members are for.
// This is stored at offset 0x23EC from the 'zone struct'
// (which I haven't detailed yet but is at location 020FB3F0
// in the USA version, and whose pointer is returned by func_02012fe4). 
// The JPN is definitely different, probably because of character buffers
// being a different size. I haven't looked at it properly yet.
// For some hints, look at the function func_020a3a34 (USA). It seems to populate
// this class based on a calling TreasureMapMetadata.
class ActiveGrottoClass
{
public:
 char maybeDiscoveryState; // was 2 in an uncleared grotto and 3 in cleared
    char maybeMapType;
    // Stored in the DQ9 string encoding. I think it's 12 bytes, but could
    // be padding at the end/secretly used for something else but was always
    // zero when I looked at it.
    char discoveredBy[12];
    char clearedBy[12];
    char probablyPadding1[2];
    // not sure about this, seemed correct for a couple of grottos but the
    // z coordinate was off by 8192. 
    int entranceZoneID;
    int entranceX;
    int entranceY;
    int entranceZ;
    unsigned char mapLocation;
    char mapImageName[16]; // e.g. "mapt_005"
    char unknown2[15];
    short seed; // might technically be unsigned
    unsigned char quality;
    char padding2[1];
    char environ; // caves, ruins, ice, water, fire as 1,2,3,4,5 resp.
    char floorCount;
    char startingMonsterRank;
    char bossID; // 1 to 12
    // these are probably chars, or maybe a short followed by 12 chars.
    // the first byte is typically quite large (0xD8 in usugura)
    // and the rest are around 1-10
    char unknown3[14];
    char prefix;
    char suffix;
    char maybeLocale; // needs more checking
    unsigned char level;
    char unknown; // was always 1 in the grottos I checked

    char buffer1[64]; // seems to hold the grotto name without level
    char buffer2[8]; // seems to hold the grotto level string (e.g. "Lv. 1")
    char buffer3[64]; // seems to hold full grotto name
    // seems to hold full grotto name too. Probably not just a buffer, though
    // in my testing the rest was all 0s.
    char buffer4[217];

#ifdef jpn
    char unknown_jp[0x20];
#endif

    FloorMap floorMap;
    FloorMapGenerator* pGenerator;
    int floorWidth, floorHeight;

    bool CalculateFloorMap(int floor, int width, int height, FloorMap* pFloorMap);

    int CalculateAndStoreFloorWidth(int floor);
    int CalculateAndStoreFloorHeight(int floor);
    int GetFloorMonsterRank(int floor) const;
    // doesn't modify this class but it does advance the A-table
    int RandomizeChestRank(int floor);
    int GetActiveGrottoEnviron() const;

    // here we get functions func_02090180 and func_020901fc.
    // The former might be calculating the number of floors.
    // The latter seems to return a pointer to one of the buffers, but unsure
    // what it does with it (probably fills it with the name or something)

    // could totally be static, but in practice gets called with a this pointer
    unsigned short GetActiveGrottoSeed() const;
    void BlankFunction() const; // does literally nothing. why does this exist

    // func_02090294 belongs here. I have no clue what it does, but it seems
    // to run only when you start following a treasure map. It writes to byte
    // 630 (the last variable in this class).

    void Clear();
};