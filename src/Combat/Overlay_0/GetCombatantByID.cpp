#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
struct CombatantStruct* GetCombatantByID(int unused, int id) {
	return GetCombatantFromList(GetBattleStruct(),id);
}