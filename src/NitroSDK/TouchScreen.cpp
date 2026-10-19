#include "NitroSDK/TouchScreen.h"
#include "System/IPC.h"
#include "System/Interrupts.h"
#include "System/BiosData.h"
#include "NitroSDK/HWRegisters.h"
#include "System/Input.h"

struct LinearCoefficients
{
    int offset;
    int scale;
    int invScale;
};

struct Struct_021117b0
{
    unsigned short isInitialized;
    char pad_2[2];
    TouchScreenCallback callback;
    // this isn't used anywhere but is written to.
    // maybe other functions used it but were removed by the linker?
    TouchScreenSample scratchSample;
    unsigned short currentSampleIndex;
    unsigned short numActiveSamples;
    TouchScreenSample* sampleBuffer;
    unsigned short sampleCapacity;
    char pad_1a[2];
    LinearCoefficients coeffs[2];
    unsigned short isCalibrated;
    // also written to but never read
    unsigned short unknown_36;
    unsigned short errors;
    volatile unsigned short currentOperations;
} extern data_021117b0;

extern "C"
{
    // abort() or similar
    void func_020c9be0();
}

int ComputeCalibrationCoefficients(TouchScreenCalibration* dst,
    unsigned short inX1, unsigned short inY1, unsigned short outX1, unsigned short outY1,
    unsigned short inX2, unsigned short inY2, unsigned short outX2, unsigned short outY2);

static inline void QueueDivision(unsigned int numerator, unsigned int denominator)
{
    DIVCNT = 0;
    DIV_NUMER_LOW = numerator;
    DIV_DENOM_64 = denominator;
}

static inline unsigned int GetDivisionResult()
{
    while (DIVCNT & 0x8000) {}
    return DIV_RESULT_LOW;
}

static inline void CopyTouchScreenSample(TouchScreenSample* dst, const TouchScreenSample* src)
{
    struct RawFormat { unsigned short x[4]; };
    *(RawFormat*)dst = *(const RawFormat*)src;
}

static inline void GetTouchScreenSampleFromArm7(TouchScreenSample* dst)
{
    union
    {
        struct
        {
            unsigned int x : 12;
            unsigned int y : 12;
            unsigned int pressed : 1;
            unsigned int unknown_6 : 2;
        };
        unsigned short words[2];
    } arm7data;

    arm7data.words[0] = ARM7_TOUCHDATAA;
    arm7data.words[1] = ARM7_TOUCHDATAB;

    dst->x = arm7data.x;
    dst->y = arm7data.y;
    dst->isPressed = (unsigned char)arm7data.pressed;
    dst->missingCoordFlags = (unsigned char)arm7data.unknown_6;
}

void TouchScreenIPCCommandReceiveProc(unsigned int a, unsigned int b, unsigned int c)
{
    unsigned short arg2Word = b;
    unsigned short arg2WordUpper = (arg2Word & 0x7f00) >> 8;

    if (c != 0)
    {
        data_021117b0.errors |= 1 << arg2WordUpper;
        if (data_021117b0.callback != NULL)
            data_021117b0.callback(arg2WordUpper, 4, 0);
        return;
    }

    if (arg2WordUpper == 16)
    {
        data_021117b0.currentSampleIndex++;
        if (data_021117b0.currentSampleIndex >= data_021117b0.sampleCapacity)
            data_021117b0.currentSampleIndex = 0;

        GetTouchScreenSampleFromArm7(&data_021117b0.sampleBuffer[data_021117b0.currentSampleIndex]);

        if (data_021117b0.callback != NULL)
            data_021117b0.callback(arg2WordUpper, 0, data_021117b0.currentSampleIndex);
        return;
    }

    if (!(b & 0x1000000))
        return;

    unsigned short callbackArg2;
    switch (arg2Word & 0xff)
    {
    case 0:
        switch (arg2WordUpper)
        {
        case 0:
            GetTouchScreenSampleFromArm7(&data_021117b0.scratchSample);
            data_021117b0.unknown_36 = 0;
            break;
        case 1:
            data_021117b0.unknown_36 = 2;
            break;
        case 2:
            data_021117b0.unknown_36 = 0;
            break;
        }

        data_021117b0.currentOperations &= ~(1 << arg2WordUpper);
        if (data_021117b0.callback != NULL)
            data_021117b0.callback(arg2WordUpper, 0, 0);
        break;
    case 4:
        callbackArg2 = 3;
        goto common;
    case 2:
        callbackArg2 = 1;
        goto common;
    case 3:
        callbackArg2 = 2;
        goto common;
    common:
        data_021117b0.errors |= 1 << arg2WordUpper;
        data_021117b0.currentOperations &= ~(1 << arg2WordUpper);

        if (data_021117b0.callback != NULL)
            data_021117b0.callback(arg2WordUpper, callbackArg2, 0);
        break;
    case 1:
    default:
        func_020c9be0();
        break;
    }
}

void InitializeTouchSystem()
{
    if (data_021117b0.isInitialized)
        return;

    data_021117b0.isInitialized = true;
    InitializeInterProcessorCommunication();
    data_021117b0.currentSampleIndex = 0;
    data_021117b0.callback = 0;
    data_021117b0.sampleBuffer = 0;
    data_021117b0.unknown_36 = 0;
    data_021117b0.isCalibrated = false;
    data_021117b0.currentOperations = 0;
    data_021117b0.errors = 0;

    while (!IsIPCCommandHandlerRegistered(6, IPCSide_Arm7)) {}
    SetArm9IPCCommandHandler(6, TouchScreenIPCCommandReceiveProc);
}

CBool GetTouchScreenCalibration(TouchScreenCalibration* dst)
{
    const FirmwareSettings* settings = &CURRENT_FIRMWARE_SETTINGS;

    unsigned short inX1 = settings->rawCalibrationPointX1;
    unsigned short inY1 = settings->rawCalibrationPointY1;
    unsigned short inX2 = settings->rawCalibrationPointX2;
    unsigned short inY2 = settings->rawCalibrationPointY2;

    unsigned char outX1 = settings->calibrationPixelX1;
    unsigned char outY1 = settings->calibrationPixelY1;
    unsigned char outX2 = settings->calibrationPixelX2;
    unsigned char outY2 = settings->calibrationPixelY2;

    if ((inX1 == 0 && inX2 == 0 && inY1 == 0 && inY2 == 0) ||
        ComputeCalibrationCoefficients(dst, inX1, inY1, outX1, outY1, inX2, inY2, outX2, outY2))
    {
        dst->xOffset = 0;
        dst->yOffset = 0;
        dst->xScale = 0;
        dst->yScale = 0;
        return true;
    }
    return true;
}

void SaveTouchScreenCalibration(TouchScreenCalibration* tsc)
{
    if (tsc == NULL)
    {
        data_021117b0.isCalibrated = false;
        return;
    }

    int priorState = DisableIRQInterrupts();

    if (tsc->xScale != 0)
    {
        QueueDivision(1 << 28, tsc->xScale);
        data_021117b0.coeffs[0].offset = tsc->xOffset;
        data_021117b0.coeffs[0].scale = tsc->xScale;
        data_021117b0.coeffs[0].invScale = GetDivisionResult();
    }
    else
    {
        data_021117b0.coeffs[0].offset = 0;
        data_021117b0.coeffs[0].scale = 0;
        data_021117b0.coeffs[0].invScale = 0;
    }

    if (tsc->yScale != 0)
    {
        QueueDivision(1 << 28, tsc->yScale);
        data_021117b0.coeffs[1].offset = tsc->yOffset;
        data_021117b0.coeffs[1].scale = tsc->yScale;
        data_021117b0.coeffs[1].invScale = GetDivisionResult();
    }
    else
    {
        data_021117b0.coeffs[1].offset = 0;
        data_021117b0.coeffs[1].scale = 0;
        data_021117b0.coeffs[1].invScale = 0;
    }

    SetIRQInterruptState(priorState);
    data_021117b0.isCalibrated = true;
}

void SetTouchScreenCallback(TouchScreenCallback callback)
{
    int priorState = DisableIRQInterrupts();
    data_021117b0.callback = callback;
    SetIRQInterruptState(priorState);
}

void EnableTouchScreen(int arg1, int arg2, TouchScreenSample* data, int numEntries)
{
    data_021117b0.sampleBuffer = data;
    data_021117b0.currentSampleIndex = 0;
    data_021117b0.numActiveSamples = arg2;
    data_021117b0.sampleCapacity = numEntries;

    for (unsigned int i = 0; i < numEntries; i++)
        data_021117b0.sampleBuffer[i].isPressed = false;

    int priorState = DisableIRQInterrupts();

    CBool success;
    if (SendCommandToArm7(6, (arg2 & 0xff) | 0x02000100, false) < 0)
        success = false;
    else if (SendCommandToArm7(6, arg1 | 0x01010000, false) < 0)
        success = false;
    else
        success = true;
   
    if (!(unsigned char)success)
    {
        SetIRQInterruptState(priorState);
        data_021117b0.errors |= 2;
        if (data_021117b0.callback != NULL)
            data_021117b0.callback(1, 4, 0);
        return;
    }

    data_021117b0.currentOperations |= 2;
    data_021117b0.errors &= ~2;
    SetIRQInterruptState(priorState);
}

void DisableTouchScreen()
{
    int priorState = DisableIRQInterrupts();

    CBool success;
    if (SendCommandToArm7(6, 0x03000200, false) >= 0)
        success = true;
    else
        success = false;

    if (!success)
    {
        SetIRQInterruptState(priorState);
        data_021117b0.errors |= 4;
        if (data_021117b0.callback != NULL)
            data_021117b0.callback(2, 4, 0);
        return;
    }

    data_021117b0.currentOperations |= 4;
    data_021117b0.errors &= ~4;
    SetIRQInterruptState(priorState);
}

void GetTouchScreenData(TouchScreenSample* output)
{
    int i;
    
    output->missingCoordFlags = 3;
    int current = data_021117b0.currentSampleIndex;
    if (data_021117b0.numActiveSamples == 1 || data_021117b0.sampleCapacity == 1)
    {
        CopyTouchScreenSample(output, &data_021117b0.sampleBuffer[current]);
        return;
    }

    for (i = 0; i < data_021117b0.numActiveSamples && i < data_021117b0.sampleCapacity - 1; i++)
    {
        int sourceIndex = current - i;
        if (sourceIndex < 0)
            sourceIndex += data_021117b0.sampleCapacity;
        
        TouchScreenSample* entry = &data_021117b0.sampleBuffer[sourceIndex];
        if (!entry->isPressed)
        {
            CopyTouchScreenSample(output, entry);
            return;
        }

        if ((output->missingCoordFlags & TOUCH_SCREEN_SAMPLE_MISSING_X) &&
            !(entry->missingCoordFlags & TOUCH_SCREEN_SAMPLE_MISSING_X))
        {
            output->x = entry->x;
            if (i != 0)
                output->missingCoordFlags &= ~TOUCH_SCREEN_SAMPLE_MISSING_X;
        }

        if ((output->missingCoordFlags & TOUCH_SCREEN_SAMPLE_MISSING_Y) &&
            !(entry->missingCoordFlags & TOUCH_SCREEN_SAMPLE_MISSING_Y))
        {
            output->y = entry->y;
            if (i != 0)
                output->missingCoordFlags &= ~TOUCH_SCREEN_SAMPLE_MISSING_Y;
        }

        if (output->missingCoordFlags == 0)
        {
            output->isPressed = true;
            return;
        }
    }
    output->isPressed = true;
}

void GetTouchScreenDataAsPixels(TouchScreenSample* output)
{
    GetTouchScreenData(output);
    ConvertTouchScreenSampleToPixels(output, output);
}

unsigned short GetCurrentTouchScreenSampleIndex()
{
    return data_021117b0.currentSampleIndex;
}

int ComputeCalibrationCoefficients(TouchScreenCalibration* dst,
    unsigned short inX1, unsigned short inY1, unsigned short outX1, unsigned short outY1,
    unsigned short inX2, unsigned short inY2, unsigned short outX2, unsigned short outY2)
{
    if (inX1 >= 0x1000 || inY1 >= 0x1000 || inX2 >= 0x1000 || inY2 >= 0x1000)
        return 1;
    
    if (outX1 >= 256 || outX2 >= 256 || outY1 >= 192 || outY2 >= 192)
        return 1;
    
    if (outX1 == outX2 || outY1 == outY2 || inX1 == inX2 || inY1 == inY2)
        return 1;

    int rawWidth = inX1 - inX2;
    int width = outX1 - outX2;

    int priorState = DisableIRQInterrupts();

    QueueDivision(rawWidth << 8, width);

    int inDeltaY = inY1 - inY2;
    int outDeltaY = outY1 - outY2;

    int ratio = GetDivisionResult();
    QueueDivision(inDeltaY << 8, outDeltaY);

    if (ratio >= 0x8000 || ratio < -0x8000)
    {
        SetIRQInterruptState(priorState);
        return 1;
    }
    dst->xScale = ratio;

    int offset = (short)((((inX1 + inX2) << 8) - ((outX1 + outX2) * dst->xScale)) >> 7);
    if (offset >= 0x8000 || offset < -0x8000)
    {
        SetIRQInterruptState(priorState);
        return 1;
    }
    dst->xOffset = offset;

    ratio = GetDivisionResult();
    SetIRQInterruptState(priorState);

    if (ratio >= 0x8000 || ratio < -0x8000)
        return 1;
    dst->yScale = ratio;

    ratio = (short)((((inY1 + inY2) << 8) - ((outY1 + outY2) * dst->yScale)) >> 7);
    if (ratio >= 0x8000 || ratio < -0x8000)
        return 1;
    
    dst->yOffset = ratio;

    return 0;
}

void ConvertTouchScreenSampleToPixels(TouchScreenSample* dst, const TouchScreenSample* src)
{
    if (!data_021117b0.isCalibrated)
    {
        CopyTouchScreenSample(dst, src);
        return;
    }

    LinearCoefficients* coeffs = data_021117b0.coeffs;
    dst->isPressed = src->isPressed;
    dst->missingCoordFlags = src->missingCoordFlags;

    if (!src->isPressed)
    {
        dst->x = 0;
        dst->y = 0;
        return;
    }

    dst->x = (((long long)(src->x * 4) - coeffs[0].offset) * coeffs[0].invScale) >> 22;
    if ((short)dst->x < 0)
        dst->x = 0;
    else if ((short)dst->x > 255)
        dst->x = 255;

    dst->y = (((long long)(src->y * 4) - coeffs[1].offset) * coeffs[1].invScale) >> 22;
    if ((short)dst->y < 0)
        dst->y = 0;
    else if ((short)dst->y > 191)
        dst->y = 191;
}

void WaitForTouchScreenOperations(int mask)
{
    while (data_021117b0.currentOperations & mask) {}
}

int GetTouchScreenErrors(int mask)
{
    return data_021117b0.errors & mask;
}