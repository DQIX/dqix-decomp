#include <globaldefs.h>
#include "World/ZonePredicates.h"

// usa: func_0201b588
bool IsGrottoZone(int zoneID)
{
    return zoneID >= 40001 && zoneID <= 41505;
}

// usa: func_0201b5b0
bool IsMainGrottoFloorZone(int zoneID)
{
    // grotto floor n in environ E has id
    // 40000 + 20*E + n (0 <= E <= 4)
    return zoneID >= 40001 && zoneID <= 40096;
}

// usa: func_0201b5d8
bool IsGrottoBossFloorZone(int zoneID)
{
    return zoneID >= 41101 && zoneID <= 41505;
}
