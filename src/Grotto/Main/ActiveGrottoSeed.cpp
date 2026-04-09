#include "Grotto/Main/ActiveGrottoData.h"
#include "Combat/Main/BattleList.h"

// USA: func_02090268
// JPN: func_02090b88
unsigned short ActiveGrottoClass::GetActiveGrottoSeed() const
{
    GrottoStruct* grottoData = GetGrottoStruct(GetBattleStruct());

    if (grottoData->activeMapData.GetMapType() == TreasureMapType_Legacy)
        return 0;

    return grottoData->activeMapData.SeedOrMinTurns;
}

void ActiveGrottoClass::BlankFunction() const
{
}