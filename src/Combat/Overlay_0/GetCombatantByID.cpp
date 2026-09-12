#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#include "GameState/GameState.h"

struct GameObject* GetCombatantByID(int unused, int id) {
	return GameState::GetInstance()->GetCombatantByIndex(id);
}