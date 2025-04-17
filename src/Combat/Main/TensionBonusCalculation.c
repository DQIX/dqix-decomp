#include <globaldefs.h>
ARM float CalculateTensionBonus(int tension, int attackerLevel) {
    float levelMultiplier = attackerLevel / 10;
    levelMultiplier = 1.0f + levelMultiplier;
    return levelMultiplier * tension;
}