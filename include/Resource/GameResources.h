#pragma once

#include "Memory/SafeAllocator.h"

// sizeof == 0x44c8, or 0x4218 in JPN version.
// An instance is allocated in func_ov030_021d8a40 (USA), which is where
// much of the current type information comes from.
struct GameResources
{
    unsigned int flags_00;
    unsigned int flags_04;
    unsigned int flags_08;
    float mainBrightness; // 0xc
    int mainBrightnessTarget; // 0x10
    int mainBrightnessTimeRemaining; // 0x14
    float subBrightness; // 0x18
    int subBrightnessTarget; // 0x1c
    int subBrightnessTimeRemaining; // 0x20
    bool mainBrightnessLocked; // 0x24
    bool subBrightnessLocked; // 0x25
    bool mainBrightnessDirty; // 0x26
    bool subBrightnessDirty; // 0x27
    bool allowBrightnessApply; // 0x28

    char unk_29[0x38 - 0x29];

    SafeAllocator allocator_array_38[17];
    SafeAllocator lootableContainerAllocator_18c_;

#if defined(usa)
    SafeAllocator allocator_array_1a0[15];
    char unk_2cc[0xe70];
#elif defined(jpn)
    SafeAllocator allocator_array_1a0[11];
    char unk_2cc[0xcb0];
#endif

    SafeAllocator allocator_113c; // JPN: +0xf2c
    char unk_1150[0x70];
    SafeAllocator allocator_11c0; // JPN: +0xfb0
    char unk_11d4[0x70];
    SafeAllocator allocator_1244; // JPN: +0x1034
    char unk_1258[0x70];

    struct Substruct_12C8
    {
        char unk_0[4];

        SafeAllocator allocator_array_4[10];

        char unk_cc[0x460];

        SafeAllocator allocator_52c;
        SafeAllocator allocator_540;
        SafeAllocator allocator_554;

        char unk_568[0x70];

        SafeAllocator allocator_5d8;
    } substruct_array_12c8[4]; // jpn: 0x10b8

    char unknown_2a78[0x94]; // jpn: 0x2868

    SafeAllocator allocator_2b0c; // jpn: 0x28fc
    char unknown_2b20[0x70]; // jpn: 0x2910

    struct Substruct_2b90
    {
        char unk_0[0x88];
    } substruct_array_2b90[0x12]; // jpn: 0x2980

    char unknown_3520[0x88]; // jpn: 0x3310
    char unknown_35a8[0x120]; // jpn: 0x3398

    void* unknown_ptr_36c8; // jpn: 0x34b8
    void* unknown_ptr_36cc; // jpn: 0x34bc
    void* unknown_ptr_36d0; // jpn: 0x34c0

    char unk_36d4[4];   // jpn: 0x34c4

    void* unknown_ptr_36d8; // jpn: 0x34c8

    // +0x36dc USA / +0x34cc JPN
    // func_ov017_0219219c(resources, index) returns a pointer into this
    // array when index is 0..3:
    // func_ov017_02191b58 subsequently reads the selected byte.
    unsigned char unknown_state_36dc[4];

    char unknown_36e0; // jpn: 0x34d0

    // +0x36e1 USA / +0x34d1 JPN
    // func_ov017_0219219c maps indices 0xC0..0xC7 to:
    unsigned char unknown_state_36e1[8];

    char unknown_36e9[0x13]; // jpn: 0x34d9

    void* unknown_ptr_array_36fc[7]; // jpn: 0x34ec
    void* unknown_ptr_3718; // jpn: 0x3508
    void* unknown_ptr_array_371c[8]; // jpn: 0x350c

    struct Substruct_373c
    {
        char unk_0[0x48];
    } substruct_array_373c[0xc]; // jpn: 0x352c

    struct Substruct_3a9c
    {
#if defined(usa)
        char unk_0[0x18];
#elif defined(jpn)
        char unk_0[0x14];
#endif
    } substruct_array_3a9c[4]; // jpn: 0x388c

    void* unknown_ptr_array_3afc[0x33]; // jpn: 0x38dc

    struct Substruct_3bc8
    {
        char unk_0[0x14];
    } substruct_array_3bc8[3]; // jpn: 0x39a8

    struct Substruct_3c04
    {
        char unknown[0x28];
    } substruct_array_3c04[4]; // jpn: 0x39e4

    void* unknown_ptr_array_3ca4[4]; // jpn: 0x3a84
    char unknown_3cb4[0x3d4]; // jpn: 0x3a94

    void* unknown_ptr_array_4088[3]; // jpn: 0x3e68

    char unknown_4094[0x130]; // jpn: 0x3e74

    void* unknown_ptr_41c4; // jpn: 0x3fa4

    char unknown_41c8[0x158]; // jpn: 0x3fa8

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
    char unknown_432c[0x60]; // jpn: 0x410c
#endif

    void* unknown_ptr_441c; // jpn: 0x416c
    char unk_4420[0x6c]; // jpn: 0x4170

    // usa: 0x448c, jpn: 0x41dc
    struct TreasureMapLanguageDataOffsets* pTMapLanguageOffsets;

    char unk_4490[0x34]; // jpn: 0x41e0
    void* unknown_ptr_44c4; // jpn: 0x4214
};

// sizeof == 0x44
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

// GameResources global-instance accessors (Overlay 17)

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
