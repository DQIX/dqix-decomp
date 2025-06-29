#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#include "Combat/Main/CombatCalculations.h"
#include "Combat/Overlay_0/GetCombatantByID.h"
ARM int GetAttackBaseDamage(int* param_1, int attackerId, int defenderId, int* param_4, int* param_5) {
    unsigned int* RNG;
	struct BattleStruct* battleStruct = GetBattleStruct();
    RNG = (unsigned int*)param_1[4];
    if (param_5 == NULL) {
        float defendersDefense;
        struct CombatantStruct* attacker = GetCombatantByID((int)RNG, attackerId);
        struct CombatantStruct* defender = GetCombatantByID(param_1[4], attackerId);
        if (attacker == NULL || defender == NULL) {
            return 0;
        }
        //defendersDefense = defender->baseStats->defense; // needs struct fleshed out
        if ((param_4[4] & 0x200) != 0) {
            defendersDefense = defender->currentStats->primaryStats.defense;
        }
        defendersDefense *= ((float*)param_1)[7];
        defendersDefense = RoundUp(defendersDefense);
        return CalculatePhysicalDamage(defendersDefense, attacker->currentStats->primaryStats.attack, (int*)param_1[4]);
    } else {
        // depends on param_5, don't know what it is
    }
}