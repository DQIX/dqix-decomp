#include "Grotto/Main/GrottoStruct.h"
#include "Combat/Main/BattleList.h"

// USA: func_02011738
// JPN: func_020114a8
GrottoStruct* GetGrottoStruct(GameState* state)
{
#ifdef jpn
    return (GrottoStruct*)((char*)state + 0x6184);
#else
    return (GrottoStruct*)((char*)state + 0x63E4);
#endif
}