#include "Grotto/Main/GrottoStruct.h"

#ifdef jpn
#define GROTTO_NAME_DATA_OFFSET 0x6180
#else
#define GROTTO_NAME_DATA_OFFSET 0x63e0
#endif

// USA: func_02011644
// JPN: func_020113b4
unsigned char* GetTreasureMapLanguageData(BattleStruct* battle)
{
    return *(unsigned char**)((char*)battle + GROTTO_NAME_DATA_OFFSET);
}

// USA: func_02011650
// JPN: func_020113b4
void SetTreasureMapLanguageDataPtr(BattleStruct* battle, unsigned char* to)
{
    *(unsigned char**)((char*)battle + GROTTO_NAME_DATA_OFFSET) = to;
}