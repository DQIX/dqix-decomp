#pragma once

#include "Memory/SafeAllocator.h"

// sizeof == 0x44c8, or 0x4218 in JPN version.
//
// An instance is allocated in func_ov030_021d8a40 (USA), which is where
// much of the current type information comes from.
struct GameResources {
    // +0x00
    unsigned int flags_00;

    // +0x04
    unsigned int flags_04;

    // +0x08
    unsigned int flags_08;

    // +0x0c
    float mainBrightness;

    // +0x10
    int mainBrightnessTarget;

    // +0x14
    int mainBrightnessTimeRemaining;

    // +0x18
    float subBrightness;

    // +0x1c
    int subBrightnessTarget;

    // +0x20
    int subBrightnessTimeRemaining;

    // +0x24
    bool mainBrightnessLocked;

    // +0x25
    bool subBrightnessLocked;

    // +0x26
    bool mainBrightnessDirty;

    // +0x27
    bool subBrightnessDirty;

    // +0x28
    bool allowBrightnessApply;

    char unknown_29[0x38 - 0x29];

    // +0x38
    SafeAllocator allocator_array_38[17];

    // +0x18c
    SafeAllocator lootableContainerAllocator_18c_;

#if defined(usa)
    SafeAllocator allocator_array_1a0[15];
    char unknown_2cc[0xe70];
#elif defined(jpn)
    SafeAllocator allocator_array_1a0[11];
    char unknown_2cc[0xcb0];
#endif

    SafeAllocator allocator_113c; // JPN: +0xf2c
    char unknown_1150[0x70];

    SafeAllocator allocator_11c0; // JPN: +0xfb0
    char unknown_11d4[0x70];

    SafeAllocator allocator_1244; // JPN: +0x1034
    char unknown_1258[0x70];

    // USA: +0x12c8
    // JPN: +0x10b8
    struct Substruct_12C8 {
        char unknown_0[4];

        SafeAllocator allocator_array_4[10];

        char unknown_cc[0x460];

        SafeAllocator allocator_52c;
        SafeAllocator allocator_540;
        SafeAllocator allocator_554;

        char unknown_568[0x70];

        SafeAllocator allocator_5d8;

    } substruct_array_12c8[4];

    // USA: +0x2a78
    // JPN: +0x2868
    char unknown_2a78[0x94];

    // USA: +0x2b0c
    // JPN: +0x28fc
    SafeAllocator allocator_2b0c;

    // USA: +0x2b20
    // JPN: +0x2910
    char unknown_2b20[0x70];

    // USA: +0x2b90
    // JPN: +0x2980
    struct Substruct_2b90 {
        char unknown[0x88];

    } substruct_array_2b90[0x12];

    // USA: +0x3520
    // JPN: +0x3310
    char unknown_3520[0x88];

    // USA: +0x35a8
    // JPN: +0x3398
    char unknown_35a8[0x120];

    void* unknown_ptr_36c8; // JPN: +0x34b8
    void* unknown_ptr_36cc; // JPN: +0x34bc
    void* unknown_ptr_36d0; // JPN: +0x34c0

    char unknown_36d4[4];   // JPN: +0x34c4

    void* unknown_ptr_36d8; // JPN: +0x34c8

    // +0x36dc USA / +0x34cc JPN
    // func_ov017_0219219c(resources, index) returns a pointer into this
    // array when index is 0..3:
    //
    // func_ov017_02191b58 subsequently reads the selected byte.
    unsigned char unknown_state_36dc[4];

    // +0x36e0 USA / +0x34d0 JPN
    char unknown_36e0;

    // +0x36e1 USA / +0x34d1 JPN
    // func_ov017_0219219c maps indices 0xC0..0xC7 to:
    unsigned char unknown_state_36e1[8];

    // +0x36e9 .. +0x36fb
    char unknown_36e9[0x13];

    // +0x36fc
    // JPN: +0x34ec
    void* unknown_ptr_array_36fc[7];

    void* unknown_ptr_3718;
    void* unknown_ptr_array_371c[8];

    // USA: +0x373c
    // JPN: +0x352c
    struct Substruct_373c {
        char unknown[0x48];

    } substruct_array_373c[0xc];

    // USA: +0x3a9c
    // JPN: +0x388c
    struct Substruct_3a9c {
#if defined(usa)
        char unknown[0x18];
#elif defined(jpn)
        char unknown[0x14];
#endif

    } substruct_array_3a9c[4];

    // USA: +0x3afc
    // JPN: +0x38dc
    void* unknown_ptr_array_3afc[0x33];

    // USA: +0x3bc8
    // JPN: +0x39a8
    struct Substruct_3bc8 {
        char unknown[0x14];

    } substruct_array_3bc8[3];

    // USA: +0x3c04
    // JPN: +0x39e4
    struct Substruct_3c04 {
        char unknown[0x28];

    } substruct_array_3c04[4];

    // USA: +0x3ca4
    // JPN: +0x3a84
    void* unknown_ptr_array_3ca4[4];

    char unknown_3cb4[0x3d4];

    // USA: +0x4088
    // JPN: +0x3e68
    void* unknown_ptr_array_4088[3];

    char unknown_4094[0x130];

    // USA: +0x41c4
    // JPN: +0x3fa4
    void* unknown_ptr_41c4;

    // USA: +0x41c8 .. +0x431f
    char unknown_41c8[0x158];

    // +0x4320 (USA)
    // It must remain a 4-byte type rather than C++ bool because the game
    // accesses it with 32-bit LDR/STR instructions.
    //
    // Initialized to 1 during GameResources initialization.
    //
    // func_ov017_02195530:
    //     field_4320 = 1;
    //
    // func_ov017_02195540:
    //     field_4320 = 0;
    //
    // func_ov000_021688dc sets this field to 1 shortly before clearing
    // flags_08::BATTLE_ACTIVE.
    unsigned int field_4320;

    // +0x4324
    char unknown_4324[4];

    // +0x4328
    // JPN: +0x4108
    void* unknown_ptr_4328;

#if defined(usa)
    char unknown_432c[0xf0];
#elif defined(jpn)
    // JPN: +0x410c
    char unknown_432c[0x60];
#endif

    // USA: +0x441c
    // JPN: +0x416c
    void* unknown_ptr_441c;

    char unknown_4420[0x6c];

    // USA: +0x448c
    // JPN: +0x41dc
    struct TreasureMapLanguageDataOffsets* pTMapLanguageOffsets;

    char unknown_4490[0x34];

    // USA: +0x44c4
    void* unknown_ptr_44c4;
};

// sizeof == 0x44
//
// Needs to eventually be moved back to the grotto-related code.
struct TreasureMapLanguageDataOffsets {
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
    unsigned int grottoBossDrops;  // flat, size 0x0C per boss
    unsigned int legacyBossDrops;  // flat, size 0x5C per boss
    unsigned int legacyBossData;
};

// --------------------------------------------------------------------------
// GameResources global-instance accessors (Overlay 17)
// --------------------------------------------------------------------------

#ifdef jpn
    // JPN accessors still use their original symbols.
    #define SetGameResources func_ov017_0218c1c0
    #define GetGameResources func_ov017_0218c1d0
extern "C" void SetGameResources(GameResources* gameResources);
extern "C" GameResources* GetGameResources();
#else
// USA: 0x0218b5a0
void SetGameResources(GameResources* gameResources);

// USA: 0x0218b5b0
GameResources* GetGameResources();
#endif
