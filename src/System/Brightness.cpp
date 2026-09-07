#include "Combat/Main/UnknownContext.h"
#include <globaldefs.h>

extern "C" ARM int IsSubBrightnessTransitionActive(UnknownContext* context)
{
    return context->subBrightnessTimeRemaining > 0;
}

extern "C" ARM int IsBrightnessTransitionActive(UnknownContext* context)
{
    int active;

    if (context->mainBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active != 0)
        goto active_transition;

    if (context->subBrightnessTimeRemaining > 0)
        active = 1;
    else
        active = 0;

    if (active == 0)
        goto no_transition;

active_transition:
    return 1;

no_transition:
    return 0;
}

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