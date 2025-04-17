#include <globaldefs.h>
ARM float CalculateCritRate(int deftness, float accessoryBonus, float bookBonus, float skillBonus, unsigned char hitCount) {
    float baseChanceFloat;
    float critChance;
    float hitCountFloat;
    float minimumChance = 2.0f;
    int baseChance = (deftness - 150) * 0x10000 >> 0x10;
    if (baseChance < 0) {
        baseChance = 0;
    }
    baseChanceFloat = baseChance;
    baseChanceFloat = 0.01f * baseChanceFloat;
    hitCountFloat = hitCount;
    hitCountFloat = 1.0f / hitCountFloat; // results in multiplying by 0.5 for 2 hits, 0.333~ for 3, etc. kinda makes crits on multihit moves bad though
    baseChanceFloat = minimumChance + baseChanceFloat;
    critChance = accessoryBonus + baseChanceFloat;
    critChance = bookBonus + critChance;
    critChance = skillBonus * critChance;
    return hitCountFloat * critChance;
}