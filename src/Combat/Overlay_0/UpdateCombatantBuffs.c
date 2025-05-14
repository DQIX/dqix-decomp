#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#include "Combat/Main/CombatCalculations.h"

void UpdateCombatantAttack(int unused, int combatantId) {
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
    attackBuff = combatant->currentStats->buffPack;
    attack = combatant->baseStats->primaryStats.attack;
    attackBuff <<= BUFF_BITS_ATTACK;
    attackBuff >>= BUFF_BITS_START;
    buffMultiplier = CalculateAttackBuffMultiplier(attackBuff);
    buffedAttack = buffMultiplier * attack;
    if (maxAttack < buffedAttack) {
        combatant->currentStats->primaryStats.attack = maxAttack;
    } else {
        combatant->currentStats->primaryStats.attack = buffedAttack;
    }
}

void UpdateCombatantDefense(int unused, int combatantId) {
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
    defenseBuff = combatant->currentStats->buffPack;
    defense = combatant->baseStats->primaryStats.defense;
    defenseBuff <<= BUFF_BITS_DEFENSE;
    defenseBuff >>= BUFF_BITS_START;
    buffMultiplier = CalculateDefenseBuffMultiplier(defenseBuff);
    buffedDefense = buffMultiplier * defense;
    if (maxDefense < buffedDefense) {
        combatant->currentStats->primaryStats.defense = maxDefense;
    } else {
        combatant->currentStats->primaryStats.defense = buffedDefense;
    }
}

void UpdateCombatantAgility(int unused, int combatantId) {
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
    agilityMultiplier = CalculateAgilityBuffMultiplier((combatant->currentStats->buffPack << BUFF_BITS_AGILITY) >> BUFF_BITS_START);
    agilityBuffed = agilityMultiplier * agility;
    if (maxAgility < agilityBuffed) {
        combatant->currentStats->primaryStats.agility = 999;
    } else {
        combatant->currentStats->primaryStats.agility = agilityBuffed;
    }
}

void UpdateCombatantCharm(int unused, int combatantId) {
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
    charmBuffValue = (combatant->currentStats->buffPack << BUFF_BITS_CHARM) >> BUFF_BITS_START;
    charm = (unsigned int)(combatant->baseStats->primaryStats.packedStats << STAT_BITS_CHARM) >> STAT_BITS_START;
    charmMultiplier = CalculateCharmBuffMultiplier(charmBuffValue);
    charmBuffed = charmMultiplier * charm;
    if (maxCharm < charmBuffed) {
        combatant->currentStats->primaryStats.packedStats = (combatant->currentStats->primaryStats.packedStats & 0xFFFFFC00) | maxCharm;
    } else {
        combatant->currentStats->primaryStats.packedStats = (combatant->currentStats->primaryStats.packedStats & 0xFFFFFC00) | (charmBuffed & STAT_MASK_CHARM);
    }
}

void UpdateCombatantMagicalMight(int unused, int combatantId) {
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
    magicalMightBuff = (combatant->currentStats->buffPack << BUFF_BITS_MAGICAL_MIGHT) >> BUFF_BITS_START;
    magicalMight = (unsigned int)(combatant->baseStats->primaryStats.packedStats << STAT_BITS_MAGICAL_MIGHT) >> STAT_BITS_START;
    buffMultiplier = CalculateMagicalMightBuffMultiplier(magicalMightBuff);
    magicalMightBuffed = buffMultiplier * magicalMight;
    if (maxMagicalMight < magicalMightBuffed) {
        combatant->currentStats->primaryStats.packedStats = (combatant->currentStats->primaryStats.packedStats & 0xFFF003FF) | (maxMagicalMight << STAT_BITS_PER_STAT);
    } else {
        combatant->currentStats->primaryStats.packedStats = (combatant->currentStats->primaryStats.packedStats & 0xFFF003FF) | ((unsigned int)(magicalMightBuffed << STAT_BITS_START) >> STAT_BITS_MAGICAL_MIGHT);
    }
}

void UpdateCombatantMagicalMending(int unused, int combatantId) {
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
    magicalMendingBuff = (combatant->currentStats->buffPack << BUFF_BITS_MAGICAL_MENDING) >> BUFF_BITS_START;
    magicalMending = (unsigned int)(combatant->baseStats->primaryStats.packedStats << STAT_BITS_MAGICAL_MENDING) >> STAT_BITS_START;
    buffMultiplier = CalculateMagicalMendingBuffMultiplier(magicalMendingBuff);
    magicalMendingBuffed = buffMultiplier * magicalMending;
    if (maxMagicalMending < magicalMendingBuffed) {
        combatant->currentStats->primaryStats.packedStats = (combatant->currentStats->primaryStats.packedStats & 0xc00ffffF) | (maxMagicalMending << (STAT_BITS_PER_STAT*2));
    } else {
        combatant->currentStats->primaryStats.packedStats = (combatant->currentStats->primaryStats.packedStats & 0xc00ffffF) | ((unsigned int)(magicalMendingBuffed << STAT_BITS_START) >> STAT_BITS_MAGICAL_MENDING);
    }
}

void ApplyCombatantBuffs(int unused, int combatantId) {
    UpdateCombatantAttack(unused, combatantId);
    UpdateCombatantDefense(unused, combatantId);
    UpdateCombatantAgility(unused, combatantId);
    UpdateCombatantCharm(unused, combatantId);
    UpdateCombatantMagicalMight(unused, combatantId);
    UpdateCombatantMagicalMending(unused, combatantId);
}