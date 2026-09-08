#pragma once

#include "std_library_functions.h"

// used for linear transitions between alpha values
struct AlphaTween
{
    // Uses range 0-65535 to get higher precision than 0-31 while still
    // being fixed precision in computations
    uint16_t current;
    // Uses range 0-65535 to get higher precision than 0-31 while still
    // being fixed precision in computations
    uint16_t target;
    float changePerTick;

    float GetTarget() const;
    void SetCurrentValue(float to);
    void Reset();

    // Returns the new value, as a float in the range 0-31
    float Advance(int numTicks);
    // Set up the struct to tween towards the specified alpha value
    // (in the range 0-31), from the current value, over the specified number
    // of ticks
    void ConfigureTween(int targetAlpha, int duration);
};