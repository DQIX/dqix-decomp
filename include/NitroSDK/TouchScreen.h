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
    unsigned short isHeld;
    unsigned short missingCoordFlags;
};

#define TOUCH_SCREEN_SAMPLE_MISSING_X 1
#define TOUCH_SCREEN_SAMPLE_MISSING_Y 2

typedef void (*TouchScreenCallback)(int, unsigned char, unsigned char);

void InitializeTouchSystem();

CBool GetTouchScreenCalibration(TouchScreenCalibration* out);
void SaveTouchScreenCalibration(TouchScreenCalibration* tsc);

void SetTouchScreenCallback(TouchScreenCallback callback);

void EnableTouchScreen(int arg1, int arg2, TouchScreenSample* sampleBuffer, int sampleCount);
void DisableTouchScreen();

void GetTouchScreenData(TouchScreenSample* output);
void GetTouchScreenDataAsPixels(TouchScreenSample* output);

unsigned short GetCurrentTouchScreenSampleIndex();

void ConvertTouchScreenSampleToPixels(TouchScreenSample* sampleA, const TouchScreenSample* sampleB);

void WaitForTouchScreenOperations(int mask);
int GetTouchScreenErrors(int mask);