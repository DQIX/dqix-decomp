#pragma once

struct ModifiableCombatStats {
	unsigned short currHP;
	unsigned short currMP;
	unsigned short maxHP;
	unsigned short maxMP;
	unsigned short attack;
	unsigned short defense;
	unsigned short agility;
};

struct CombatantStruct {
	unsigned short flags;
	char unk[0x132];
	void* baseStats; // TODO: holds more general info than just stats
	struct ModifiableCombatStats* currentStats; // includes things like buffs being applied
};

struct BattleStruct {
	int unk0;
	int unk4;
	struct CombatantStruct* combatantList[0xe9]; // TODO: validate this size as well as this struct as a whole
};
struct BattleStruct* GetBattleStruct();
struct CombatantStruct* GetCombatantFromList(struct BattleStruct* battleStruct, int id);