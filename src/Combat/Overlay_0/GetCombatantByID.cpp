#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#include "GameState.h"

struct CombatantStruct* GetCombatantByID(int unused, int id) {
	return GetCombatantFromList((BattleStruct*)GameState::GetInstance(),id);
}