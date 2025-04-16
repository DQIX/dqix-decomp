extern float func_02074388(int,float,float);
extern int data_02108ddc;
// properly decompiled variant
_Pragma("thumb off") float CalculatePhysicalDamage(int attack, int defense, int variance) {
    float atkAsFloat;
    float defAsFloat;
    float varAmnt;
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
            defAsFloat = atkAsFloat / 16.0f;
            defAsFloat = 0.0f - defAsFloat;
            varAmnt = atkAsFloat / 16.0f;
            defAsFloat = func_02074388(variance, defAsFloat, varAmnt);
            varAmnt = func_02074388(variance, -1.0f, 1.0f);
            atkAsFloat += defAsFloat;
            atkAsFloat += varAmnt;
        }
    }
    if (atkAsFloat < 0) {
        atkAsFloat = 0;
    }
	return atkAsFloat;
}