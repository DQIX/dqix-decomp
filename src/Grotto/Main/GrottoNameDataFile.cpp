#include "GameState/GameState.h"

// USA: func_02011644
// JPN: func_020113b4
unsigned char* GameState::GetTreasureMapLanguageData()
{
    return treasureMapLanguageData_;
}

// USA: func_02011650
// JPN: func_020113b4
void GameState::SetTreasureMapLanguageDataPtr(unsigned char* to)
{
    treasureMapLanguageData_ = to;
}