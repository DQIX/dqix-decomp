#pragma once

// Absolutely terrible name, but I don't know nearly enough about it
// to go with anything more sensible. I do know it is of size 0x44C8
// (maybe minus up to 3 bytes) and seems to hold a huge number of pointers
// to other data. In JPN version the size is 0x4218

struct Struct_ov017_44C8
{
#ifdef jpn
    char unknown_0[0x41dc];
#else
    char unknown_0[0x448c];
#endif
    struct TreasureMapLanguageDataOffsets* pTMapLanguageOffsets;
    char unknown_4490[0x38];
};

// size 0x44
struct TreasureMapLanguageDataOffsets
{
    unsigned int bossRangesByQuality;
    unsigned int bossIDsAndWeights;
    unsigned int environs;
    unsigned int prefixRangesByMonsterRank;
    unsigned int prefixNames;
    unsigned int floorRangesByQuality;
    unsigned int unknown_18; 
    unsigned int startingMonsterRanksByQuality;
    unsigned int mapLocations;
    unsigned int seeminglyChestRanksByMonsterRank;
    unsigned int localeRankRangesByFloorCount;
    unsigned int localeNames;
    unsigned int suffixRangesByBoss;
    unsigned int suffixNames;
    unsigned int grottoBossDrops; // flat, size 0x0C per boss
    unsigned int legacyBossDrops; // flat, size 0x5C per boss
    unsigned int legacyBossData;
};

// Will rename this after getting a better idea of what the struct is
#ifdef jpn
#define func_ov017_0218b5b0 func_ov017_0218c1d0
#endif

// This is the second function in overlay 17 (the first one stores this pointer).
// So it's possible the struct is some sort of overall struct for the overlay.
extern "C" Struct_ov017_44C8* func_ov017_0218b5b0();