#pragma once

struct Random {
    /* +00 */ unsigned long long lcg_state;
    /* +08 */ unsigned long long lcg_multiplier;
    /* +10 */ unsigned long long lcg_increment;
    /* +18 */ char const* name;
    /* +1C */ unsigned char unk_1C;
};

#ifdef jpn
#define data_02108ddc data_02108d20
#endif

extern struct Random data_02108ddc;

/* get instance of the global "BT" ("Hoimi table") random */
struct Random* GetBTRandom(void);

/* Random constructor */
struct Random* CreateRandom(struct Random* random, char const* name, unsigned char unk_1C);

void InitRandom(struct Random* random, unsigned long long lcg_state, char const* name, unsigned char unk_1C);

void SeedRandom64(struct Random* random, unsigned long long lcg_state);

void SeedRandom(struct Random* random, unsigned long long lcg_state);

void SeedRandom32(struct Random* random, unsigned int stateHi, unsigned int stateLo);

unsigned int GetRandomStateHi(struct Random* random);

unsigned int GetRandomStateLo(struct Random* random);

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
