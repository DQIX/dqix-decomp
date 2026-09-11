#pragma once

#include "Combat/Main/BattleList.h"
#include "Resource/GameResources.h"
#include "World/Object3D.h"

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
    int playerObjectIndex_;
    char unk_3b0[0x7ff4 - 0x3b0];

public:
    static GameState* GetInstance();

    // Index into the object array, but only return it if its obj3D.unk_0
    // has bit 0x80 set. In practice this seems to be for enemies in battle
    // and party members universally. In a fight with multiple enemies, you can
    // clear this bit on one enemy and kill the others, and the battle will end
    // prematurely.
    GameObject* GetCombatantByIndex(int idx);
};