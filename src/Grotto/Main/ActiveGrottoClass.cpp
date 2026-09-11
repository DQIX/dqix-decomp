#include "Grotto/Main/ActiveGrottoClass.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Main/TreasureMapDataStructs.h"
#include "GameState.h"
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

    // Not sure exactly what these do but it involves loading/unloading
    // data/tmap/tdata.gp2
    void func_020a3720();
    void func_020a395c();
}

// USA: func_0209fe68
// JPN: func_02090780
bool ActiveGrottoClass::CalculateFloorMap(int floor, int width, int height, FloorMap* pFloorMap)
{
    if (pFloorMap == NULL)
        pFloorMap = &floorMap_;
    
    pFloorMap->Initialize(width, height);
    pGenerator_->Initialize();
    pGenerator_->pFloorMap = pFloorMap;
    pGenerator_->seed = GetActiveGrottoSeed();
    pGenerator_->Calculate(floor, 0);
    pFloorMap->ComputeAdjacencyData();
    return true;
}

// USA: func_0208fed0
// USA: func_020907e8
int ActiveGrottoClass::CalculateAndStoreFloorWidth(int floor)
{
    if (floor >= 0 && floor <= 4)
        floorWidth_ = GetMapDimensionFromRange(10, 14, floor);
    else if (floor >= 5 && floor <= 8)
        floorWidth_ = GetMapDimensionFromRange(12, 15, floor);
    else if (floor >= 9 && floor <= 12)
        floorWidth_ = GetMapDimensionFromRange(14, 16, floor);
    else
        floorWidth_ = 16;

    return floorWidth_;
}

// USA: func_0208ff5c
// JPN: func_02090874
int ActiveGrottoClass::CalculateAndStoreFloorHeight(int floor)
{
    if (floor >= 0 && floor <= 4)
        floorHeight_ = GetMapDimensionFromRange(10, 14, floor);
    else if (floor >= 5 && floor <= 8)
        floorHeight_ = GetMapDimensionFromRange(12, 15, floor);
    else if (floor >= 9 && floor <= 12)
        floorHeight_ = GetMapDimensionFromRange(14, 16, floor);
    else
        floorHeight_ = 16;

    return floorHeight_;
}

// USA: func_0208ffe8
// JPN: func_02090900
int ActiveGrottoClass::GetFloorMonsterRank(int floor) const
{
    GrottoStruct* grotto = GetGrottoStruct(GameState::GetInstance());
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
    GrottoStruct* grotto = GetGrottoStruct(GameState::GetInstance());

    if (grotto->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 1;
    else
        return grotto->activeEnviron;
}

// USA: func_02090180
// JPN: func_02090a98
int ActiveGrottoClass::GetFloorCount() const
{
    GrottoStruct* grotto = GetGrottoStruct(GameState::GetInstance());
    void* zone = func_02012fe4();
    if (!func_0201b588(*(unsigned short*)zone))
        return 0;

    if (grotto->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 0;

    if (overallMapData_.discoveryState_ == DiscoveryState_Invalid)
    {
        func_020a3720();
        DetailedTreasureMapData data;
        ExportDetailedTreasureMapData(&grotto->activeMapData, &data, 1, 0);
        func_020a395c();
        return data.regular_.floorCount_;
    }

    return overallMapData_.regular_.floorCount_;
}

// USA: func_020901fc
// JPN: func_02090b14
const char* ActiveGrottoClass::GetPopupName() const
{
    GrottoStruct* grotto = GetGrottoStruct(GameState::GetInstance());
    if (overallMapData_.discoveryState_ != DiscoveryState_Invalid)
    {
        if (overallMapData_.mapType_ == TreasureMapType_Legacy)
            return overallMapData_.legacy_.popupName_;
        else
            return overallMapData_.regular_.popupName_;
    }    

    // Returning a temporary, could something go wrong here?
    // Would have to call another function after this which uses the stack.
    func_020a3720();
    DetailedTreasureMapData data;
    ExportDetailedTreasureMapData(&grotto->activeMapData, &data, 1, 0);
    func_020a395c();

    if (data.mapType_ == TreasureMapType_Legacy)
        return data.legacy_.popupName_;
    else
        return data.regular_.popupName_;
}

// USA: func_02090268
// JPN: func_02090b88
unsigned short ActiveGrottoClass::GetActiveGrottoSeed() const
{
    GrottoStruct* grottoData = GetGrottoStruct(GameState::GetInstance());

    if (grottoData->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 0;

    return grottoData->activeMapData.SeedOrMinTurns;
}

// USA: func_02090290
// JPN: func_02090bb0
DetailedTreasureMapData* ActiveGrottoClass::GetDetailedData()
{
    return &overallMapData_;
}