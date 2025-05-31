#include <globaldefs.h>
#include "System/Random.h"

extern char const data_020f0d5c[]; // "(no-name)"

extern struct Random data_02108ddc; /* note: initialized by usa:func_020e60c0, in .init */

ARM struct Random* GetBTRandom(void) {
    return &data_02108ddc;
}

ARM struct Random* CreateRandom(struct Random* random, char const* name, unsigned char unk_1C) {
    /* NOTE: if this were C++, this would be a constructor */
    InitRandom(random, 0uLL, name, unk_1C);
    return random;
}

ARM void InitRandom(struct Random* random, unsigned long long lcg_state, char const* name, unsigned char unk_1C) {
    if (name == NULL) {
        name = data_020f0d5c;
    }

    random->unk_1C = unk_1C;
    random->name = name;

    SeedRandom(random, lcg_state);
}

ARM void SeedRandom64(struct Random* random, unsigned long long lcg_state) {
    SeedRandom(random, lcg_state);
}

ARM void SeedRandom(struct Random* random, unsigned long long lcg_state) {
    unsigned long long multiplier = 0x5D588B656C078965uLL;
    unsigned long long increment = 0x0000000000269EC3uLL;

    random->lcg_state = lcg_state;
    random->lcg_multiplier = multiplier;
    random->lcg_increment = increment;
}

ARM void SeedRandom32(struct Random* random, unsigned int stateHi, unsigned int stateLo) {
    /* a bit ugly... */
    SeedRandom(random, (((unsigned long long) stateHi << 32u) & 0xFFFFFFFF00000000uLL) | (stateLo & 0xFFFFFFFFull));
}

ARM unsigned int GetRandomStateHi(struct Random* random) {
    return (random->lcg_state >> 32u) & 0xFFFFFFFFu;
}

ARM unsigned int GetRandomStateLo(struct Random* random) {
    return random->lcg_state & 0xFFFFFFFFu;
}

ARM unsigned int NextRandom(struct Random* random) {
    /* <https://en.wikipedia.org/wiki/Linear_congruential_generator> */

    random->lcg_state = random->lcg_multiplier * random->lcg_state + random->lcg_increment;
    return random->lcg_state >> 32u;
}

ARM int NextRandomMax(struct Random* random, int maximum) {
    unsigned char saved_unk_1C;
    int result;

    if (maximum <= 0) {
        return 0;
    }

    saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = maximum * NextRandomFloat01(random);

    if (result >= maximum) {
        result = maximum - 1;
    }

    random->unk_1C = saved_unk_1C;

    return result;
}

ARM float NextRandomFloat01(struct Random* random) {
    float result;

    unsigned char saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = NextRandom(random) / (double) 0xFFFFFFFF;

    random->unk_1C = saved_unk_1C;

    return result;
}

ARM float NextRandomFloatBetween(struct Random* random, float minimum, float maximum) {
    unsigned char saved_unk_1C;
    float result;

    saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = (maximum - minimum) * NextRandomFloat01(random) + minimum;

    random->unk_1C = saved_unk_1C;

    return result;
}

ARM float NextRandomFloatScaled(struct Random* random, float minimum, float maximum, int digits) {
    unsigned char saved_unk_1C;
    int scale;
    int minimum_int;
    int minimum_max;
    float result;

    saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    scale = 1;

    {
        int i;
        for (i = 0; i < digits; i++) {
            scale = scale * 10;
        }
    }

    minimum_int = minimum * scale;
    minimum_max = maximum * scale;

    result = (float) NextRandomBetween(random, minimum_int, minimum_max) / scale;

    random->unk_1C = saved_unk_1C;

    return result;
}

ARM int NextRandomBetween(struct Random* random, int minimum, int maximum) {
    unsigned char saved_unk_1C;
    int result;

    saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = NextRandomMax(random, (maximum - minimum) + 1) + minimum;

    random->unk_1C = saved_unk_1C;

    return result;
}
