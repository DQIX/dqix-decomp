#include <globaldefs.h>
#ifdef usa
extern float func_02074388(int,float,float);
#endif
#ifdef jpn
extern float func_02075514(int,float,float);
#define func_02074388 func_02075514
#define data_02108ddc data_02108d20
#endif
extern int data_02108ddc;
ARM int RoundUp(float attack) {
	return 0.5f + attack;
}
ARM float CalculatePhysicalDamage(int attack, int defense, int* variance) {
    float atkAsFloat;
    float defAsFloat;
	if (!variance) {
		variance = &data_02108ddc;
	}
    atkAsFloat = attack;
    defAsFloat = defense;
    defAsFloat /= 2.0f;
    atkAsFloat -= defAsFloat;
    atkAsFloat /= 2.0f;
    if (atkAsFloat <= 0) {
        atkAsFloat = 0;
    } else {
        float minimumDamage = attack;
        minimumDamage /= 16.0f;
        if (atkAsFloat <= minimumDamage) {
            atkAsFloat = attack;
            atkAsFloat /= 16.0f;
            atkAsFloat = func_02074388(variance, 0.0f, atkAsFloat);

        } else {
            float flatVariance;
            float percentageVarianceMaximum;
            float percentageVariance;
            float percentageVarianceMinimum = atkAsFloat / 16.0f;
            percentageVarianceMinimum = 0.0f - percentageVarianceMinimum;
            percentageVarianceMaximum = atkAsFloat / 16.0f;
            percentageVariance = func_02074388(variance, percentageVarianceMinimum, percentageVarianceMaximum);
            flatVariance = func_02074388(variance, -1.0f, 1.0f);
            atkAsFloat += percentageVariance;
            atkAsFloat += flatVariance;
        }
    }
    if (atkAsFloat < 0) {
        atkAsFloat = 0;
    }
	return atkAsFloat;
}
ARM float CalculateAttackBuffMultiplier(signed char buffLevel) {
    return 1.0f + 0.25f * buffLevel;
}

ARM float CalculateDefenseBuffMultiplier(signed char buffLevel) {
    // increases defense by 50% for each positive level; for -1 do 50% defense reducation, for -2 do 75% defense reduction.
    float levelFloat;
    float multiplier;
    if (buffLevel >= 0) {
        levelFloat = buffLevel;
        multiplier = levelFloat * 0.5f;
        return 1.0f + multiplier;
    }
    levelFloat = buffLevel+1;
    multiplier = levelFloat * 0.25f;
    multiplier = multiplier - 0.5f;
    return 1.0f + multiplier;
}

ARM float CalculateAgilityBuffMultiplier(signed char buffLevel) {
    // increases defense by 50% for each positive level; for -1 do 50% defense reducation, for -2 do 75% defense reduction. Identical to defense
    float levelFloat;
    float multiplier;
    if (buffLevel >= 0) {
        levelFloat = buffLevel;
        multiplier = levelFloat * 0.5f;
        return 1.0f + multiplier;
    }
    levelFloat = buffLevel+1;
    multiplier = levelFloat * 0.25f;
    multiplier = multiplier - 0.5f;
    return 1.0f + multiplier;
}

ARM float CalculateCharmBuffMultiplier(signed char buffLevel) {
    return (buffLevel < 0) ? 1.0f : 1.0f + 0.5f * buffLevel;
}

ARM float CalculateMagicalMightBuffMultiplier(signed char buffLevel) {
    return 1.0f + 0.5f * buffLevel;
}

ARM float CalculateMagicalMendingBuffMultiplier(signed char buffLevel) {
    return 1.0f + 0.5f * buffLevel;
}