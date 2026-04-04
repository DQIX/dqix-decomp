#include "Grotto/Main/FloorMapGenerator.h"
#include "std_library_functions.h"
#include <globaldefs.h>



// USA: func_020925b8
// JPN: func_02092ed8
ARM void FloorMapGenerator::Initialize()
{
    seed = -1;
    pFloorMap = NULL;
    partitionRectCount = 1;
    boundaryCount = 0;
    roomCount = 0;
    mainRoomAssigned = false;

    memset(partitionRects, 0, sizeof(partitionRects));
    memset(boundaries, 0, sizeof(boundaries));
    memset(rooms, 0, sizeof(rooms));
}

// USA: func_02092614
// JPN: func_02092f34
ARM int FloorMapGenerator::RandRange(int minimum, int maximum) const
{
    if (minimum == maximum)
        return minimum;
    
    return minimum + (rand() % (maximum - minimum + 1));
}

