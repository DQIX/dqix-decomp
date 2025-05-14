#pragma once

#define BUFF_BITS_START 0x1d
#define BUFF_BITS_ATTACK 0x1d
#define BUFF_BITS_DEFENSE 0x1a
#define BUFF_BITS_AGILITY 0x17
#define BUFF_BITS_CHARM 0x14
#define BUFF_BITS_MAGICAL_MIGHT 0x11
#define BUFF_BITS_MAGICAL_MENDING 0xe

#define STAT_BITS_START 0x16
#define STAT_BITS_CHARM 0x16
#define STAT_MASK_CHARM 0x3FF
#define STAT_BITS_MAGICAL_MIGHT 0xC
#define STAT_BITS_MAGICAL_MENDING 0x2
#define STAT_BITS_PER_STAT 0xA

struct PrimaryCombatStats {
	unsigned short currHP;
	unsigned short currMP;
	unsigned short maxHP;
	unsigned short maxMP;
	unsigned short attack;
	unsigned short defense;
	unsigned short agility;
	unsigned short unk;
	int packedStats;
};

struct BaseCombatStats {
	char unk[0x2C];
	struct PrimaryCombatStats primaryStats;
};

struct ModifiableCombatStats {
	struct PrimaryCombatStats primaryStats;// 0xE
	char unk1[0x44];
	int buffPack;
};

struct CombatantStruct {
	unsigned short flags;
	char unk[0x132];
	struct BaseCombatStats* baseStats; // TODO: holds more general info than just stats
	struct ModifiableCombatStats* currentStats; // includes things like buffs being applied
};

struct BattleStruct {
	int unk0;
	int unk4;
	struct CombatantStruct* combatantList[0xe9]; // TODO: validate this size as well as this struct as a whole
};
struct BattleStruct* GetBattleStruct();
struct CombatantStruct* GetCombatantFromList(struct BattleStruct* battleStruct, int id);