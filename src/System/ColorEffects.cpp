#include "System/Graphics.h"
#include "System/ColorEffects.h"
#include <asmhacks.h>

// despite seeming like an sdk function, this matches better at O2 than O2,p
//#pragma optimize_for_size off

void ColorEffect_ConfigureAlphaBlend(intptr_t control,
    unsigned int pixel1Source, unsigned int pixel2Source,
    unsigned int pixel1Alpha, unsigned int pixel2Alpha)
{
    *(volatile unsigned int*)(control) = pixel1Source | 0x40 | (pixel2Source << 8)
        | (pixel1Alpha | (pixel2Alpha << 8)) << 16;
}

void ColorEffect_ConfigureBrightnessAdjust(intptr_t control, unsigned int pixelSource, int adjust)
{
    if (adjust >= 0)
    {
        *(volatile unsigned short*)(control) = pixelSource | 0x80;
        *(volatile unsigned short*)(control + 4) = adjust;
        return;
    }
    else
    {
        DECLARE_ASM_NOP();
        *(volatile unsigned short*)(control) = pixelSource | 0xc0;
        *(volatile unsigned short*)(control + 4) = -adjust;
    }
}

void ColorEffect_ChangeBrightness(intptr_t control, int newBrightness)
{
    unsigned short ctrlValue = *(volatile unsigned short*)control;
    int currentEffect = ctrlValue & 0xc0;
    if (newBrightness < 0)
    {
        // swap from brightness increase to brightness decrease if needed
        if (currentEffect == 0x80)
            *(volatile unsigned short*)control = (ctrlValue & ~0xc0) | 0xc0;
        *(volatile unsigned short*)(control + 4) = -newBrightness;
    }
    else
    {
        if (currentEffect == 0xc0)
            *(volatile unsigned short*)control = (ctrlValue & ~0xc0) | 0x80;
        *(volatile unsigned short*)(control + 4) = newBrightness;
    }
}