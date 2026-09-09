#pragma once

// Absolutely terrible name, but I don't know nearly enough about it
// to go with anything more sensible. I do know it is of size 0x44C8
// (maybe minus up to 3 bytes) and seems to hold a huge number of pointers
// to other data. In JPN version the size is 0x4218.
//
// The struct is allocated by func_ov030_021d8a40 (usa) which gives some
// information about the type of its members. (This is the first function
// in that overlay).

#include "Memory/SafeAllocator.h"

struct Struct_ov017_44C8
{
    char unknown_0[0x38];

    SafeAllocator allocator_array_38[17];
    SafeAllocator lootableContainerAllocator_18c_;
#if defined(usa)
    SafeAllocator allocator_array_1a0[15];
    char unknown_2cc[0xe70];
#elif defined(jpn)
    SafeAllocator allocator_array_1a0[11];
    char unknown_2cc[0xcb0];
#endif

    SafeAllocator allocator_113c; // jpn: offset f2c
    char unknown_1150[0x70];
    SafeAllocator allocator_11c0; // jpn: offset fb0
    char unknown_11d4[0x70];
    SafeAllocator allocator_1244; // jpn: offset 1034
    char unknown_1258[0x70];

    struct Substruct_12C8 // jpn: offset 10b8, ending at 2868
    {
        char unknown_0[4];
        SafeAllocator allocator_array_4[10];
        char unknown_cc[0x460];
        SafeAllocator allocator_52c;
        SafeAllocator allocator_540;
        SafeAllocator allocator_554;
        char unknown_568[0x70];
        SafeAllocator allocator_5d8;
    } substruct_array_12c8[4];
    char unknown_2a78[0x94]; // jpn: offset 2868
    SafeAllocator allocator_2b0c; // jpn: offset 28fc

    char unknown_2b20[0x70]; // jpn: offset 2910
    struct Substruct_2b90 // jpn: offset 2980
    {
        char unknown[0x88];
    } substruct_array_2b90[0x12];
    char unknown_3520[0x88]; // jpn: offset 3310
    char unknown_35a8[0x120]; // jpn: offset 3398

    void* unknown_ptr_36c8; // jpn: offset 34b8
    void* unknown_ptr_36cc; // jpn: offset 34bc
    void* unknown_ptr_36d0; // jpn: offset 34c0
    char unknown_36d4[4]; // jpn: offset 34c4
    void* unknown_ptr_36d8; // jpn: offset 34c8
    char unknown_36dc[0x20]; // jpn: offset 34cc
    void* unknown_ptr_array_36fc[7]; // jpn: offset 34ec
    void* unknown_ptr_3718;
    void* unknown_ptr_array_371c[8];
    struct Substruct_373c // jpn: offset 352c
    {
        char unknown[0x48];
    } substruct_array_373c[0xc];
    struct Substruct_3a9c // jpn: offset 388c
    {
#if defined(usa)
        char unknown[0x18];
#elif defined(jpn)
        char unknown[0x14];
#endif
    } substruct_array_3a9c[4];
    void* unknown_ptr_array_3afc[0x33]; // jpn: offset 38dc
    struct Substruct_3bc8 // jpn: offset 39a8
    {
        char unknown[0x14];
    } substruct_array_3bc8[3];
    struct Substruct_3c04 // jpn: offset 39e4
    {
        char unknown[0x28];
    } substruct_array_3c04[4];
    void* unknown_ptr_array_3ca4[4]; // jpn: offset 3a84
    char unknown_3cb4[0x3d4];
    void* unknown_ptr_array_4088[3]; // jpn: offset 3e68
    char unknown_4094[0x130];
    void* unknown_ptr_41c4; // jpn: offset 3fa4
    char unknown_41c8[0x160];
    void* unknown_ptr_4328; // jpn: offset 4108

#if defined(usa)
    char unknown_432c[0xf0];
#elif defined(jpn)
    char unknown_432c[0x60]; // offset 410c
#endif
    void* unknown_ptr_441c; // jpn: offset 416c
    char unknown_4420[0x6c];
    struct TreasureMapLanguageDataOffsets* pTMapLanguageOffsets; // usa: offset 448c, jpn: offset 41dc
    char unknown_4490[0x34];
    void* unknown_ptr_44c4;
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