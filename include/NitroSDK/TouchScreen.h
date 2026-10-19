#pragma once

#include <std_library_functions.h>

struct TouchScreenCalibration
{
    short xOffset;
    short yOffset;
    short xScale;
    short yScale;
};

struct TouchScreenSample
{
    unsigned short x;
    unsigned short y;
    unsigned short isPressed;
    unsigned short missingCoordFlags;
};

#define TOUCH_SCREEN_SAMPLE_MISSING_X 1
#define TOUCH_SCREEN_SAMPLE_MISSING_Y 2

typedef void (*TouchScreenCallback)(int, unsigned char, unsigned char);

CBool GetTouchScreenCalibration(TouchScreenCalibration* out);
void SaveTouchScreenCalibration(TouchScreenCalibration* tsc);

void GetTouchScreenData(TouchScreenSample* output);

void ConvertTouchScreenSampleToPixels(TouchScreenSample* sampleA, const TouchScreenSample* sampleB);