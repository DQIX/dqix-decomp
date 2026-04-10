#pragma once

#include "GrottoStruct.h"
#include "FloorMapGenerator.h"

#include "DetailedTreasureMapData.h"

// Very WIP. I think in the USA version, the size of this should be
// 632 (0x278), but I don't know what the last members are for.
// This is stored at offset 0x23EC from the 'zone struct'
// (which I haven't detailed yet but is at location 020FB3F0
// in the USA version, and whose pointer is returned by func_02012fe4). 
// The JPN is 0x20 bytes larger, seemingly because of text buffers being
// different sizes.
// For some hints, look at the function func_020a3a34 (USA). It seems to populate
// this class based on a calling TreasureMapMetadata.
class ActiveGrottoClass
{
public:
    // Data not pertaining to specific floors
    DetailedTreasureMapData overallMapData;
    
    int unknown_1C4;
    FloorMap floorMap;
    FloorMapGenerator* pGenerator;
    int floorWidth, floorHeight;
    char unknown_260[3];
    int unknown_264[4];
    short unknown_274;
    char unknown_276;

    bool CalculateFloorMap(int floor, int width, int height, FloorMap* pFloorMap);

    int CalculateAndStoreFloorWidth(int floor);
    int CalculateAndStoreFloorHeight(int floor);
    int GetFloorMonsterRank(int floor) const;
    // doesn't modify this class but it does advance the A-table
    int RandomizeChestRank(int floor);
    int GetActiveGrottoEnviron() const;

    int GetFloorCount() const;
    const char* GetPopupName() const;

    unsigned short GetActiveGrottoSeed() const;
    void BlankFunction() const; // does literally nothing. why does this exist

    // func_02090294 belongs here. I have no clue what it does, but it seems
    // to run only when you start following a treasure map. It writes to byte
    // 630 (the last variable in this class).

    void Clear();
    void ClearGenerator(bool keepFloormap);
    void BlankFunction2() const; // again does literally nothing

    // Definitely not confusing at all: pass true if you *dont* want
    // to allocate the floor map's buffers, and false if you *do* want to.
    void AllocateGenerator(void* allocator, bool skipAllocateMapBuffers);

    // 'randomly' picks a dimension from within the given range using the
    // modulus trick, except the rng value is (grotto seed + floor)
    int GetMapDimensionFromRange(int minimum, int maximum, int floor) const;
};