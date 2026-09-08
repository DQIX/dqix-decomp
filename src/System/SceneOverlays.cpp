#include <globaldefs.h>
#include "System/SceneOverlays.h"

extern const SceneEntry data_020e8f20[];

extern int data_01ffd348;            // current slot bank
extern signed char data_01ffd364[];  // six slots per bank, 8 banks?

// usa: func_020a18f4
ARM bool IsSceneLoaded(unsigned int scene)
{
    bool loaded = false;

    if (scene < SCENE_COUNT)
    {
        const signed char* slots = &data_01ffd364[data_01ffd348 * 6];
        loaded = slots[data_020e8f20[scene].group] == scene;
    }

    return loaded;
}
