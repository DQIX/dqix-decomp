#include "System/Random.h"

unsigned int NextRandom(struct Random* random) {
    /* <https://en.wikipedia.org/wiki/Linear_congruential_generator> */

    random->lcg_state = random->lcg_multiplier * random->lcg_state + random->lcg_increment;
    return random->lcg_state >> 32u;
}

int NextRandomMax(struct Random* random, int maximum) {
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

float NextRandomFloat01(struct Random* random) {
    float result;

    unsigned char saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = NextRandom(random) / (double) 0xFFFFFFFF;

    random->unk_1C = saved_unk_1C;

    return result;
}

float NextRandomFloatBetween(struct Random* random, float minimum, float maximum) {
    unsigned char saved_unk_1C;
    float result;

    saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = (maximum - minimum) * NextRandomFloat01(random) + minimum;

    random->unk_1C = saved_unk_1C;

    return result;
}

float NextRandomFloatScaled(struct Random* random, float minimum, float maximum, int digits) {
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

int NextRandomBetween(struct Random* random, int minimum, int maximum) {
    unsigned char saved_unk_1C;
    int result;

    saved_unk_1C = random->unk_1C;
    random->unk_1C = 0;

    result = NextRandomMax(random, (maximum - minimum) + 1) + minimum;

    random->unk_1C = saved_unk_1C;

    return result;
}
