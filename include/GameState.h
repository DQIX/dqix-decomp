#pragma once

#include "Combat/Main/BattleList.h"
#include "Resource/GameResources.h"
#include "World/Object3D.h"
#include "Filesystem/NitroVM.h"
#include "GameState/TimeOfDay.h"
#include "Grotto/Main/GrottoStruct.h"

// Represents a party member, monster in battle, monster on the field
// or grotto boss. 
class GameObject
{
public:
    Object3D obj3D_; // might be inherited instead
    char unk_ac[0x134 - 0xac];
    BaseCombatStats* baseStats_;
    ModifiableCombatStats* currentStats_;
};

// sizeof is probably 0x7ff4 but could be 0x7ff8. (Definitely no lower/higher)
// For lower bound, look at initialize/reset function func_0200f3a4
// which writes a byte at offset 0x7ff2.
// For upper bound, note an instance of this occurs at 0x020f33d8, constructed
// in the static initializer at 0x020e5920, while data at 0x020fb3d0 is initialized
// by the next static initializer at 0x020e59c8. data_020fb3cc is explicitly written
// to so size is probably 0x7ff4
class GameState
{
public:
    GameResources* pResources_;
    char unk_4[4];
    GameObject* objects_[0xe9];
    int protagonistObjectIndex_;
    void* unknown_3b0_; // see func_020100bc, LightingManager::MaybeComputeHorizonPosition. Probably a high level camera
    unsigned int effectiveDeltaTimeMilliseconds_;
    unsigned int trueDeltaTimeMilliseconds_;
    fix16_t gameSpeed_; // effective delta time is true delta time rescaled by this
    fix32_t animationDeltaTime_; // for use with frame-based things such as nsbca
    unsigned int numTicks_;
    unsigned int currentNumTicks_;
    float dayTimer_;
    float dayLength_;
    float daySpeed_;
    CBool dayTimerRunning_;
    TimeOfDay timeOfDay_;
    float unknown_3e0_;
    char unk_3e4[4];
    uint64_t mainTimestamp_; // current timestamp - this one is used for chest timer
    uint64_t altTimestamp_; // not sure about usage

#if defined(usa)
    char unk_3f8[0x397c - 0x3f8];
#elif defined(jpn)
    char unk_3f8[0x371c - 0x3f8];
#endif

    unsigned char unknownObjectIndex_397c_; // jpn: offset 0x731c instead
    char unk_397d[0x63e0 - 0x397d];

    unsigned char* treasureMapLanguageData_;
    GrottoStruct grottoInfo_;

    char unk_6fc0[0x7ff4 - 0x6fc0];

public:
    static GameState* GetInstance();

    GameObject* GetGameObjectByIndex(int idx);
    // Like GetCombatantByIndex() but checks for bitmask 0x2 instead. This is set
    // in the same cases as 0x20, but replacing this function to always return null
    // only disables wandering monsters, while keeping whistle spawns and grotto
    // bosses in tact
    GameObject* GetMaybeWanderingMonsterByIndex(int idx);
    GameObject* GetProtagonist();
    // Seems to also be the protagonist, checks the bit at 0x397c
    GameObject* GetUnknownGameObject();
    // Like GetCombatantByIndex() but checks for bitmask 0x800 instead.
    GameObject* GetPartyMemberByIndex(int idx);
    // Like GetCombatantByIndex() but checks for bitmask 0x20 instead. In practice
    // this bit is set for monsters out of battle, and replacing this function to
    // always return null disables monster spawns, including through whistle, and
    // removes grotto bosses.
    GameObject* GetMaybeFieldMonsterByIndex(int idx);
    // Index into the object array, but only return it if its obj3D.unk_0
    // has bit 0x80 set. In practice this seems to be for enemies in battle
    // and party members universally. In a fight with multiple enemies, you can
    // clear this bit on one enemy and kill the others, and the battle will end
    // prematurely.
    GameObject* GetCombatantByIndex(int idx);

    // --- GameTime.cpp ---

    // usa: func_02010150
    void CalculateDeltaTime(uint64_t microseconds);
    // usa: func_02010208
    unsigned int GetEffectiveDeltaTime() const;
    // usa: func_02010210
    unsigned int GetTrueDeltaTime() const;
    // usa: func_02010218
    fix32_t GetAnimationDeltaTime() const;
    // usa: func_02010220
    unsigned int GetTickCount() const;
    // usa: func_02010228
    void SetGameSpeed(fix32_t speed);
    // usa: func_02010234
    fix32_t GetGameSpeed() const;
    // usa: func_02010240
    void AdvanceDayTimer();
    // usa: func_02010280
    float GetDayTimer() const;
    // usa: func_02010288
    void SetDayTimer(float to);
    // usa: func_02010354
    void SetDayTimerRunning(CBool to);
    // usa: func_0201035c
    TimeOfDay GetTimeOfDay() const;
    // usa: func_02010364
    void SetTimeOfDay(TimeOfDay);
    // usa: func_020103b4
    // used for determining inn dialogue, whether you can enter
    // Mirage Mahal/Stornway Castle etc. Not used for town music
    bool IsMorningDayOrEvening() const;

    // --- GrottoNameDataFile.cpp ---
    unsigned char* GetTreasureMapLanguageData();
    void SetTreasureMapLanguageDataPtr(unsigned char*);

    // --- GrottoStruct.cpp ---
    GrottoStruct* GetGrottoStruct();
};