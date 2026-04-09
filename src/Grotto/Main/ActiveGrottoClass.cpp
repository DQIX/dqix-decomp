#include "Grotto/Main/ActiveGrottoData.h"
#include "Combat/Main/BattleList.h"
#include <globaldefs.h>

extern "C"
{
    // RandRange but it's implemented by double arithmetic
    int func_020323c4(int minimum, int maximum);

    // class member, chooses from [min, max] by doing (rng + floor), but the
    // rng value is the map seed.
     int func_02090400(ActiveGrottoClass*, int minimum, int maximum, int floor);
}

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

int ActiveGrottoClass::CalculateAndStoreFloorWidth(int floor)
{
    if (floor >= 0 && floor <= 4)
        floorWidth = func_02090400(this, 10, 14, floor);
    else if (floor >= 5 && floor <= 8)
        floorWidth = func_02090400(this, 12, 15, floor);
    else if (floor >= 9 && floor <= 12)
        floorWidth = func_02090400(this, 14, 16, floor);
    else
        floorWidth = 16;

    return floorWidth;
}

int ActiveGrottoClass::CalculateAndStoreFloorHeight(int floor)
{
    if (floor >= 0 && floor <= 4)
        floorHeight = func_02090400(this, 10, 14, floor);
    else if (floor >= 5 && floor <= 8)
        floorHeight = func_02090400(this, 12, 15, floor);
    else if (floor >= 9 && floor <= 12)
        floorHeight = func_02090400(this, 14, 16, floor);
    else
        floorHeight = 16;

    return floorHeight;
}

int ActiveGrottoClass::GetFloorMonsterRank(int floor) const
{
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());
    if (floor < 1)
        floor = 1;

    return grotto->activeStartingMonsterRank + (floor - 1) / 4;
}

int ActiveGrottoClass::RandomizeChestRank(int floor)
{
    int monsterRank = GetFloorMonsterRank(floor);
    int chestRank = 0;
    switch (monsterRank)
    {
    case 0:
        break;
    case 1:
        chestRank = func_020323c4(1, 2);
        break;
    case 2:
        chestRank = func_020323c4(1, 2);
        break;
    case 3:
        chestRank = func_020323c4(1, 3);
        break;
    case 4:
        chestRank = func_020323c4(1, 4);
        break;
    case 5:
        chestRank = func_020323c4(2, 5);
        break;
    case 6:
        chestRank = func_020323c4(2, 6);
        break;
    case 7:
        chestRank = func_020323c4(3, 7);
        break;
    case 8:
        chestRank = func_020323c4(3, 8);
        break;
    case 9:
        chestRank = func_020323c4(4, 9);
        break;
    case 10:
        chestRank = func_020323c4(5, 9);
        break;
    case 11:
        chestRank = func_020323c4(1, 10);
        break;
    case 12:
        chestRank = func_020323c4(4, 10);
        break;
    }
    return chestRank;
}

int ActiveGrottoClass::GetActiveGrottoEnviron() const
{
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());

    if (grotto->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 1;
    else
        return grotto->activeEnviron;
}