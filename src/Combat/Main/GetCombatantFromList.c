#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
ARM int* GetCombatantFromList(struct BattleStruct *battleStruct, int combatantId) {
    int* combatant;
    if (combatantId < 0) {
        return 0;
    }
    if (combatantId >= 0xE9) {
        return 0;
    }
    combatant = battleStruct->combatantList[combatantId];
    if (combatant == 0) {
        return 0;
    }
    if (((*(unsigned short*)combatant) & 0x80) == 0) {
        combatant = 0;
    }
    return combatant;
}