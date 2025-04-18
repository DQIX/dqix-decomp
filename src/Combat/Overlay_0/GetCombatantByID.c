#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
// TODO: change to character battle struct
int* GetCombatantByID(int unused, int id) {
	return GetCombatantFromList(GetBattleStruct(),id);
}