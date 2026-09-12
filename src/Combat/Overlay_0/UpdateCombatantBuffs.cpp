#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#include "Combat/Main/CombatCalculations.h"
#include "GameState/GameState.h"

ARM void UpdateCombatantAttack(int unused, int combatantId) {
    int maxAttack;
    GameState* gameState;
    GameObject* combatant;
    int combatantIsPlayer;
    unsigned int attack;
    unsigned short buffedAttack;
    int attackBuff;
    float buffMultiplier;
    gameState = GameState::GetInstance();
    combatant = gameState->GetCombatantByIndex(combatantId);
    if (combatant == NULL) {
        return;
    }
    maxAttack = 0xFFFF;
    combatantIsPlayer = !(combatantId < 0 || combatantId > 3);
    if (combatantIsPlayer != 0) {
        maxAttack = 999;
    }
    attackBuff = combatant->currentStats_->attackBuff;
    attack = combatant->baseStats_->primaryStats.attack;
    buffMultiplier = CalculateAttackBuffMultiplier(attackBuff);
    buffedAttack = buffMultiplier * attack;
    if (maxAttack < buffedAttack) {
        combatant->currentStats_->primaryStats.attack = maxAttack;
    } else {
        combatant->currentStats_->primaryStats.attack = buffedAttack;
    }
}

ARM void UpdateCombatantDefense(int unused, int combatantId) {
    int maxDefense;
    struct GameState* gameState;
    GameObject* combatant;
    int combatantIsPlayer;
    unsigned int defense;
    unsigned short buffedDefense;
    int defenseBuff;
    float buffMultiplier;
    gameState = GameState::GetInstance();
    combatant = gameState->GetCombatantByIndex(combatantId);
    if (combatant == NULL) {
        return;
    }
    maxDefense = 0xFFFF;
    combatantIsPlayer = !(combatantId < 0 || combatantId > 3);
    if (combatantIsPlayer != 0) {
        maxDefense = 999;
    }
    defenseBuff = combatant->currentStats_->defenseBuff;
    defense = combatant->baseStats_->primaryStats.defense;
    buffMultiplier = CalculateDefenseBuffMultiplier(defenseBuff);
    buffedDefense = buffMultiplier * defense;
    if (maxDefense < buffedDefense) {
        combatant->currentStats_->primaryStats.defense = maxDefense;
    } else {
        combatant->currentStats_->primaryStats.defense = buffedDefense;
    }
}

ARM void UpdateCombatantAgility(int unused, int combatantId) {
    GameState* gameState;
    GameObject* combatant;
    unsigned int agility;
    float agilityMultiplier;
    unsigned short agilityBuffed;
    const short maxAgility = 999;
    gameState = GameState::GetInstance();
    combatant = gameState->GetCombatantByIndex(combatantId);
    if (combatant == NULL) {
        return;
    }
    agility = combatant->baseStats_->primaryStats.agility;
    agilityMultiplier = CalculateAgilityBuffMultiplier(combatant->currentStats_->agilityBuff);
    agilityBuffed = agilityMultiplier * agility;
    if (maxAgility < agilityBuffed) {
        combatant->currentStats_->primaryStats.agility = 999;
    } else {
        combatant->currentStats_->primaryStats.agility = agilityBuffed;
    }
}

ARM void UpdateCombatantCharm(int unused, int combatantId) {
    GameState *gameState = GameState::GetInstance();
    GameObject* combatant = gameState->GetCombatantByIndex(combatantId);
    float charmMultiplier;
    unsigned short charm;
    unsigned short charmBuffed;
    unsigned int charmBuffValue;
    const short maxCharm = 999;
    if (combatant == NULL) {
        return;
    }
    charmBuffValue = combatant->currentStats_->charmBuff;
    charm = combatant->baseStats_->primaryStats.charm;
    charmMultiplier = CalculateCharmBuffMultiplier(charmBuffValue);
    charmBuffed = charmMultiplier * charm;
    if (maxCharm < charmBuffed) {
        combatant->currentStats_->primaryStats.charm = maxCharm;
    } else {
        combatant->currentStats_->primaryStats.charm = charmBuffed;
    }
}

ARM void UpdateCombatantMagicalMight(int unused, int combatantId) {
    unsigned short magicalMight;
    unsigned int magicalMightBuff;
    unsigned short magicalMightBuffed;
    const short maxMagicalMight = 999;
    float buffMultiplier;
    struct GameState *gameState = GameState::GetInstance();
    GameObject* combatant = gameState->GetCombatantByIndex(combatantId);
    if (combatant == NULL) {
        return;
    }
    magicalMightBuff = combatant->currentStats_->magicalMightBuff;
    magicalMight = combatant->baseStats_->primaryStats.magicalMight;
    buffMultiplier = CalculateMagicalMightBuffMultiplier(magicalMightBuff);
    magicalMightBuffed = buffMultiplier * magicalMight;
    if (maxMagicalMight < magicalMightBuffed) {
        combatant->currentStats_->primaryStats.magicalMight = maxMagicalMight;
    } else {
        combatant->currentStats_->primaryStats.magicalMight = magicalMightBuffed;
    }
}

ARM void UpdateCombatantMagicalMending(int unused, int combatantId) {
    unsigned short magicalMending;
    unsigned int magicalMendingBuff;
    unsigned short magicalMendingBuffed;
    const short maxMagicalMending = 999;
    float buffMultiplier;
    struct GameState *gameState = GameState::GetInstance();
    GameObject*combatant = gameState->GetCombatantByIndex(combatantId);
    if (combatant == NULL) {
        return;
    }
    magicalMendingBuff = combatant->currentStats_->magicalMendingBuff;
    magicalMending = combatant->baseStats_->primaryStats.magicalMending;
    buffMultiplier = CalculateMagicalMendingBuffMultiplier(magicalMendingBuff);
    magicalMendingBuffed = buffMultiplier * magicalMending;
    if (maxMagicalMending < magicalMendingBuffed) {
        combatant->currentStats_->primaryStats.magicalMending = maxMagicalMending;
    } else {
        combatant->currentStats_->primaryStats.magicalMending = magicalMendingBuffed;
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