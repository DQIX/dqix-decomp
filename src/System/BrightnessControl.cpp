#include "Combat/Main/UnknownContext.h"
#include <globaldefs.h>

extern "C" ARM void func_0203b110(UnknownContext *context, int brightness, int duration);
extern "C" ARM void func_0203b19c(UnknownContext *context, int brightness, int duration);

extern "C" ARM void SetAndLockMainBrightness(UnknownContext *context, int brightness, int duration) {
    func_0203b110(context, brightness, duration);
    context->mainBrightnessLocked = 1;
}

extern "C" ARM void SetAndLockSubBrightness(UnknownContext *context, int brightness, int duration) {
    func_0203b19c(context, brightness, duration);
    context->subBrightnessLocked = 1;
}