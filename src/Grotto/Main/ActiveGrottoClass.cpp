#include "Grotto/Main/ActiveGrottoData.h"
#include "Combat/Main/BattleList.h"
#include <globaldefs.h>

#ifdef jpn
    #define func_020323c4 func_02031efc

    #define func_02012fe4 func_02012dac
    #define func_0201b588 func_0201b300

    #define func_020a3720 func_020a5498
    #define func_020a395c func_020a5698
    #define func_020a3a34 func_020a5770
#endif

extern "C"
{
    // RandRange but it's implemented by double arithmetic
    int func_020323c4(int minimum, int maximum);

    // Returns the 'zone struct' (still need to figure out what this contains)
    void* func_02012fe4();

    // Returns true if the value is between 40001 and 41505, respectively.
    // Most likely these are the zone IDs corresponding to grottos.
    bool func_0201b588(unsigned short zoneID);

    // No idea what these do, but they seem to be called before and after
    // each call to func_020a3a34.
    void func_020a3720();
    void func_020a395c();

    // Not sure about the last two parameters. This 'exports' a TreasureMapMetadata
    // out to a UniversalData struct. Somewhere in this it's running the code to
    // generate map names. Needs some closer investigation. (Probably a member
    // function of TreasureMapMetadata)
    bool func_020a3a34(TreasureMapMetadata*, ActiveGrottoClass::UniversalData*, bool, void*);
}

// USA: func_0209fe68
// JPN: func_02090780
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

// USA: func_0208fed0
// USA: func_020907e8
int ActiveGrottoClass::CalculateAndStoreFloorWidth(int floor)
{
    if (floor >= 0 && floor <= 4)
        floorWidth = GetMapDimensionFromRange(10, 14, floor);
    else if (floor >= 5 && floor <= 8)
        floorWidth = GetMapDimensionFromRange(12, 15, floor);
    else if (floor >= 9 && floor <= 12)
        floorWidth = GetMapDimensionFromRange(14, 16, floor);
    else
        floorWidth = 16;

    return floorWidth;
}

// USA: func_0208ff5c
// JPN: func_02090874
int ActiveGrottoClass::CalculateAndStoreFloorHeight(int floor)
{
    if (floor >= 0 && floor <= 4)
        floorHeight = GetMapDimensionFromRange(10, 14, floor);
    else if (floor >= 5 && floor <= 8)
        floorHeight = GetMapDimensionFromRange(12, 15, floor);
    else if (floor >= 9 && floor <= 12)
        floorHeight = GetMapDimensionFromRange(14, 16, floor);
    else
        floorHeight = 16;

    return floorHeight;
}

// USA: func_0208ffe8
// JPN: func_02090900
int ActiveGrottoClass::GetFloorMonsterRank(int floor) const
{
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());
    if (floor < 1)
        floor = 1;

    return grotto->activeStartingMonsterRank + (floor - 1) / 4;
}

// USA: func_02090018
// JPN: func_02090930
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

// USA: func_02090158
// JPN: func_02090a70
int ActiveGrottoClass::GetActiveGrottoEnviron() const
{
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());

    if (grotto->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 1;
    else
        return grotto->activeEnviron;
}

// USA: func_02090180
// JPN: func_02090a98
int ActiveGrottoClass::GetFloorCount() const
{
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());
    void* zone = func_02012fe4();
    if (!func_0201b588(*(unsigned short*)zone))
        return 0;

    if (grotto->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 0;

    if (universal.discoveryState == DiscoveryState_Invalid)
    {
        func_020a3720();
        UniversalData data;
        func_020a3a34(&grotto->activeMapData, &data, true, NULL);
        func_020a395c();
        return data.regularMapData.floorCount;
    }

    return universal.regularMapData.floorCount;
}

// USA: func_020901fc
// JPN: func_02090b14
const char* ActiveGrottoClass::GetPopupName() const
{
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());
    if (universal.discoveryState != DiscoveryState_Invalid)
    {
        if (universal.mapType == TreasureMapType_Legacy)
            return universal.legacyMapData.popupName;
        else
            return universal.regularMapData.popupName;
    }    

    // Returning a temporary, could something go wrong here?
    // Would have to call another function after this which uses the stack.
    func_020a3720();
    UniversalData data;
    func_020a3a34(&grotto->activeMapData, &data, true, NULL);
    func_020a395c();

    if (data.mapType == TreasureMapType_Legacy)
        return data.legacyMapData.popupName;
    else
        return data.regularMapData.popupName;
}

// USA: func_02090268
// JPN: func_02090b88
unsigned short ActiveGrottoClass::GetActiveGrottoSeed() const
{
    GrottoStruct* grottoData = GetGrottoStruct(GetBattleStruct());

    if (grottoData->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 0;

    return grottoData->activeMapData.SeedOrMinTurns;
}

// USA: func_02090290
// JPN: func_02090bb0
void ActiveGrottoClass::BlankFunction() const
{
}