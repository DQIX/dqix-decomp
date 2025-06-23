#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#include "Combat/Main/CombatCalculations.h"

ARM void UpdateCombatantAttack(int unused, int combatantId) {
    int maxAttack;
    struct BattleStruct* battleStruct;
    struct CombatantStruct* combatant;
    int combatantIsPlayer;
    unsigned int attack;
    unsigned short buffedAttack;
    int attackBuff;
    float buffMultiplier;
    battleStruct = GetBattleStruct();
    combatant = GetCombatantFromList(battleStruct, combatantId);
    if (combatant == NULL) {
        return;
    }
    maxAttack = 0xFFFF;
    combatantIsPlayer = !(combatantId < 0 || combatantId > 3);
    if (combatantIsPlayer != 0) {
        maxAttack = 999;
    }
    attackBuff = combatant->currentStats->attackBuff;
    attack = combatant->baseStats->primaryStats.attack;
    buffMultiplier = CalculateAttackBuffMultiplier(attackBuff);
    buffedAttack = buffMultiplier * attack;
    if (maxAttack < buffedAttack) {
        combatant->currentStats->primaryStats.attack = maxAttack;
    } else {
        combatant->currentStats->primaryStats.attack = buffedAttack;
    }
}

ARM void UpdateCombatantDefense(int unused, int combatantId) {
    int maxDefense;
    struct BattleStruct* battleStruct;
    struct CombatantStruct* combatant;
    int combatantIsPlayer;
    unsigned int defense;
    unsigned short buffedDefense;
    int defenseBuff;
    float buffMultiplier;
    battleStruct = GetBattleStruct();
    combatant = GetCombatantFromList(battleStruct, combatantId);
    if (combatant == NULL) {
        return;
    }
    maxDefense = 0xFFFF;
    combatantIsPlayer = !(combatantId < 0 || combatantId > 3);
    if (combatantIsPlayer != 0) {
        maxDefense = 999;
    }
    defenseBuff = combatant->currentStats->defenseBuff;
    defense = combatant->baseStats->primaryStats.defense;
    buffMultiplier = CalculateDefenseBuffMultiplier(defenseBuff);
    buffedDefense = buffMultiplier * defense;
    if (maxDefense < buffedDefense) {
        combatant->currentStats->primaryStats.defense = maxDefense;
    } else {
        combatant->currentStats->primaryStats.defense = buffedDefense;
    }
}

ARM void UpdateCombatantAgility(int unused, int combatantId) {
    struct BattleStruct* battleStruct;
    struct CombatantStruct* combatant;
    unsigned int agility;
    float agilityMultiplier;
    unsigned short agilityBuffed;
    const short maxAgility = 999;
    battleStruct = GetBattleStruct();
    combatant = GetCombatantFromList(battleStruct, combatantId);
    if (combatant == NULL) {
        return;
    }
    agility = combatant->baseStats->primaryStats.agility;
    agilityMultiplier = CalculateAgilityBuffMultiplier(combatant->currentStats->agilityBuff);
    agilityBuffed = agilityMultiplier * agility;
    if (maxAgility < agilityBuffed) {
        combatant->currentStats->primaryStats.agility = 999;
    } else {
        combatant->currentStats->primaryStats.agility = agilityBuffed;
    }
}

ARM void UpdateCombatantCharm(int unused, int combatantId) {
    struct BattleStruct *battleStruct = GetBattleStruct();
    struct CombatantStruct *combatant = GetCombatantFromList(battleStruct, combatantId);
    float charmMultiplier;
    unsigned short charm;
    unsigned short charmBuffed;
    unsigned int charmBuffValue;
    const short maxCharm = 999;
    if (combatant == NULL) {
        return;
    }
    charmBuffValue = combatant->currentStats->charmBuff;
    charm = combatant->baseStats->primaryStats.charm;
    charmMultiplier = CalculateCharmBuffMultiplier(charmBuffValue);
    charmBuffed = charmMultiplier * charm;
    if (maxCharm < charmBuffed) {
        combatant->currentStats->primaryStats.charm = maxCharm;
    } else {
        combatant->currentStats->primaryStats.charm = charmBuffed;
    }
}

ARM void UpdateCombatantMagicalMight(int unused, int combatantId) {
    unsigned short magicalMight;
    unsigned int magicalMightBuff;
    unsigned short magicalMightBuffed;
    const short maxMagicalMight = 999;
    float buffMultiplier;
    struct BattleStruct *battleStruct = GetBattleStruct();
    struct CombatantStruct *combatant = GetCombatantFromList(battleStruct, combatantId);
    if (combatant == NULL) {
        return;
    }
    magicalMightBuff = combatant->currentStats->magicalMightBuff;
    magicalMight = combatant->baseStats->primaryStats.magicalMight;
    buffMultiplier = CalculateMagicalMightBuffMultiplier(magicalMightBuff);
    magicalMightBuffed = buffMultiplier * magicalMight;
    if (maxMagicalMight < magicalMightBuffed) {
        combatant->currentStats->primaryStats.magicalMight = maxMagicalMight;
    } else {
        combatant->currentStats->primaryStats.magicalMight = magicalMightBuffed;
    }
}

ARM void UpdateCombatantMagicalMending(int unused, int combatantId) {
    unsigned short magicalMending;
    unsigned int magicalMendingBuff;
    unsigned short magicalMendingBuffed;
    const short maxMagicalMending = 999;
    float buffMultiplier;
    struct BattleStruct *battleStruct = GetBattleStruct();
    struct CombatantStruct *combatant = GetCombatantFromList(battleStruct, combatantId);
    if (combatant == NULL) {
        return;
    }
    magicalMendingBuff = combatant->currentStats->magicalMendingBuff;
    magicalMending = combatant->baseStats->primaryStats.magicalMending;
    buffMultiplier = CalculateMagicalMendingBuffMultiplier(magicalMendingBuff);
    magicalMendingBuffed = buffMultiplier * magicalMending;
    if (maxMagicalMending < magicalMendingBuffed) {
        combatant->currentStats->primaryStats.magicalMending = maxMagicalMending;
    } else {
        combatant->currentStats->primaryStats.magicalMending = magicalMendingBuffed;
    }
}

ARM void ApplyCombatantBuffs(int unused, int combatantId) {
    UpdateCombatantAttack(unused, combatantId);
    UpdateCombatantDefense(unused, combatantId);
    UpdateCombatantAgility(unused, combatantId);
    UpdateCombatantCharm(unused, combatantId);
    UpdateCombatantMagicalMight(unused, combatantId);
    UpdateCombatantMagicalMending(unused, combatantId);
}