#include <globaldefs.h>
extern float func_02074388(int,float,float);
extern int data_02108ddc;
// properly decompiled variant
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
        defAsFloat = attack;
        defAsFloat /= 16.0f;
        if (atkAsFloat <= defAsFloat) {
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