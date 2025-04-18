#include <globaldefs.h>
#include "Combat/Main/BattleList.h"
#if defined(usa) || defined(jpn)
extern struct BattleStruct data_020f33d8;
#endif
ARM struct BattleStruct* GetBattleStruct() {
	return &data_020f33d8;
}