#include <globaldefs.h>
#include "World/ZonePredicates.h"

// usa: func_0201b588
ARM bool IsGrottoZone(int zoneID)
{
    return zoneID >= 0x9c41 && zoneID <= 0x9c41 + 0x5e0;
}

// usa: func_0201b5b0
ARM bool IsMainGrottoFloorZone(int zoneID)
{
    return zoneID >= 0x9c41 && zoneID <= 0x9c41 + 0x5f;
}

// usa: func_0201b5d8
ARM bool IsGrottoBossFloorZone(int zoneID)
{
    return zoneID >= 0xa08d && zoneID <= 0xa08d + 0x194;
}
