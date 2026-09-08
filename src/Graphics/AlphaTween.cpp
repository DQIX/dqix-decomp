#include "Graphics/AlphaTween.h"

float AlphaTween::GetTarget() const
{
    return (target * 31u) / 65535.0f;
}

void AlphaTween::SetCurrentValue(float to)
{
    current = 65535.0f * (to / 31.0f);
}

void AlphaTween::Reset()
{
    memset(this, 0, sizeof(AlphaTween));
}

float AlphaTween::Advance(int numTicks)
{
    if (changePerTick == 0.0f)
        return (current * 31u) / 65535.0f;
    
    float priorValueNorm = (current * 31u) / 65535.0f;
    float targetValueNorm = (target * 31u) / 65535.0f;

    float newValueNorm = priorValueNorm + (changePerTick * numTicks);
    if (0.0f < changePerTick && targetValueNorm < newValueNorm)
    {
        changePerTick = 0.0f;
        newValueNorm = targetValueNorm;
    }
    else if (changePerTick < 0.0f && newValueNorm < targetValueNorm)
    {
        changePerTick = 0.0f;
        newValueNorm = targetValueNorm;
    }

    current = 65535.0f * (newValueNorm / 31.0f);
    return newValueNorm;
}

void AlphaTween::ConfigureTween(int targetAlpha, int duration)
{
    float initialNorm = (current * 31u) / 65535.0f;
    float targetNorm = targetAlpha;
    target = 65535.0f * (targetNorm / 31.0f);

    if (duration <= 0)
        changePerTick = 0.0f;
    else
        changePerTick = (targetNorm - initialNorm) / duration;
}