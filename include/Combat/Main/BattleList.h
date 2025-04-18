#pragma once
struct BattleStruct {
	int unk0;
	int unk4;
	int* combatantList[0xe9]; // TODO: validate this size as well as this struct as a whole
};
struct BattleStruct* GetBattleStruct();
int* GetCombatantFromList(struct BattleStruct* battleStruct, int id);