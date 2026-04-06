#include "Grotto/Main/ActiveGrottoData.h"
#include <globaldefs.h>

bool ActiveGrottoClass::CalculateFloorMap(int floor, int width, int height, FloorMap* pFloorMap)
{
    if (pFloorMap == NULL)
        pFloorMap = &floorMap;
    
    pFloorMap->Initialize(width, height);
    pGenerator->Initialize();
    pGenerator->pFloorMap = pFloorMap;
    pGenerator->seed = GetActiveGrottoSeed();
    pGenerator->Calculate(floor, 0);
    pFloorMap->ComputeAdjacencyData();
    return true;
}