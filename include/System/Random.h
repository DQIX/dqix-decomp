#pragma once

struct Random {
    /* +00 */ unsigned long long lcg_state;
    /* +08 */ unsigned long long lcg_multiplier;
    /* +10 */ unsigned long long lcg_increment;
    /* +18 */ unsigned int unk_18;
    /* +1C */ unsigned char unk_1C;
};

/* generate a random unsigned integer between 0 and 0xFFFFFFFFu (inclusive). */
unsigned int NextRandom(struct Random* random);

/* generate a random integer between 0 and maximum (exclusive).
 * if maximum <= 0, returns 0 without updating state. */
int NextRandomMax(struct Random* random, int maximum);

/* generate a random floating point number between 0.0f and 1.0f (inclusive). */
float NextRandomFloat01(struct Random* random);

/* generate a random floating point number between minimum and maximum (inclusive). */
float NextRandomFloatBetween(struct Random* random, float minimum, float maximum);

/* generate a random floating point number between minimum and maximum (inclusive) with digits precision. */
float NextRandomFloatScaled(struct Random* random, float minimum, float maximum, int digits);

/* generate a random integer between minimum and maximum (inclusive) */
int NextRandomBetween(struct Random* random, int minimum, int maximum);
