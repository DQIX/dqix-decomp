#include <globaldefs.h>
ARM float CalculateCritRate(int deftness, float accessoryBonus, float bookBonus, float skillBonus, unsigned char hitCount) {
    float minimumChance = 2.0f;

    // there is some bit shifting in the assembly here, 16 left and 16 right and those preserve the most significant bit. A conversion to short seems extremely likely
    short baseChance = (deftness - 150);
    if (baseChance < 0) {
        baseChance = 0;
    }
    float baseChanceFloat = 0.01f * (float)baseChance;    
    
    float hitCountFloat = 1.0f / float(hitCount); // results in multiplying by 0.5 for 2 hits, 0.333~ for 3, etc. kinda makes crits on multihit moves bad though
    
    baseChanceFloat = minimumChance + baseChanceFloat;

    float critChance = accessoryBonus + baseChanceFloat;
    critChance = bookBonus + critChance;
    critChance = skillBonus * critChance;
    return hitCountFloat * critChance;
}