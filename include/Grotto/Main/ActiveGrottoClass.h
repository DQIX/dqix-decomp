#pragma once

#include "GrottoStruct.h"
#include "FloorMapGenerator.h"

#include "DetailedTreasureMapData.h"
#include "Memory/SafeAllocator.h"

// sizeof == 0x278 == 632. In JPN version it's 0x20 bytes larger due
// to DetailedTreasureMapData having extra buffers.
// This is stored at offset 0x23EC in the 'zone struct'
// (which I haven't detailed yet but is at location 020FB3F0
// in the USA version, and whose pointer is returned by func_02012fe4).
// Holds data about the grotto the player is currently in, including (but not
// limited to) data about the current floor. 
class ActiveGrottoClass
{
public:
    // Data not pertaining to specific floors
    DetailedTreasureMapData overallMapData_;
    
    int unk_1c4;
    FloorMap floorMap_;
    FloorMapGenerator* pGenerator_;
    int floorWidth_, floorHeight_;
    char unknown_260_[3]; // not sure where this is read. Might not be an array
    int unknown_264_[4]; // not sure where this is read. Might not be an array
    short unknown_274_;
    char unknown_276_;

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
    DetailedTreasureMapData* GetDetailedData();

    // func_02090294 belongs here. I have no clue what it does, but it seems
    // to run only when you start following a treasure map. It writes to byte
    // 630 (the last variable in this class).

    void Clear();
    void ClearGenerator(bool keepFloormap);
    void BlankFunction2() const; // again does literally nothing

    // Definitely not confusing at all: pass true if you *dont* want
    // to allocate the floor map's buffers, and false if you *do* want to.
    void AllocateGenerator(SafeAllocator* allocator, bool skipAllocateMapBuffers);

    // 'randomly' picks a dimension from within the given range using the
    // modulus trick, except the rng value is (grotto seed + floor)
    int GetMapDimensionFromRange(int minimum, int maximum, int floor) const;
};