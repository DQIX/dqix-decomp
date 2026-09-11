#include <globaldefs.h>
#include "GameState.h"

#if defined(usa) || defined(jpn)
extern GameState data_020f33d8;
#endif

GameState* GameState::GetInstance()
{
	return &data_020f33d8;
}