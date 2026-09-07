#pragma once
#include "std_library_functions.h"

extern "C"
{
    // Pass e.g. the address of REG_BLDCNT, and both it and REG_BLDALPHA
    // (which is 2 bytes after it) will be adjusted appropriately.
    // The sources are a bitmask of which layers to use
    // (BG0, ..., BG3, OBJ, Backdrop are 0x01, ..., 0x20 respectively)
    // and the alpha values range from 0 (min) to 16 (max) inclusive.
    // For a normal alpha blend effect, the values should sum to 16
    void ColorEffect_ConfigureAlphaBlend(intptr_t control,
        unsigned int pixel1Source, unsigned int pixel2Source,
        unsigned int pixel1Alpha, unsigned int pixel2Alpha);

    // adjust can range from -16 (fully black) to 16 (fully white)
    // with 0 = no adjustment
    void ColorEffect_ConfigureBrightnessAdjust(intptr_t control,
        unsigned int pixelSource, int adjust);

    void ColorEffect_ChangeBrightness(intptr_t control, int newBrightness);
}