#include "Combat/Main/UnknownContext.h"
#include <globaldefs.h>

extern "C" ARM int GetMainBrightnessTransitionState(UnknownContext *context) {
    int active;

    if (context->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return 0;

    int state = 1;

    if (context->mainBrightnessTarget < 0) state = 2;

    if (context->mainBrightnessTarget > 0) state = 3;

    return state;
}

extern "C" ARM int GetSubBrightnessTransitionState(UnknownContext *context) {
    int active;

    if (context->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0) return 0;

    int state = 1;

    if (context->subBrightnessTarget < 0) state = 2;

    if (context->subBrightnessTarget > 0) state = 3;

    return state;
}

extern "C" ARM unsigned short GetBrightnessTransitionStates(UnknownContext* context)
{
    int mainState = GetMainBrightnessTransitionState(context);
    int subState = GetSubBrightnessTransitionState(context);

    return mainState | (subState << 8);
}