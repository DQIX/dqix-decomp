#pragma once

#include "Combat/Main/BattleList.h"

// Nineteen accessors on BattleStruct, contiguous at 0x02011518..0x0201165c.
// Each is a getter, setter, clear or test on one field; the field offsets are
// what established the tail of BattleStruct. Names carry the offset because
// none of the fields' meanings are established.

extern "C"
{

// USA: func_02011518 - bounds checked against rosterCount, -1 when out of range
int GetRosterId(struct BattleStruct* battle, unsigned int index);
// USA: func_02011538
unsigned char GetRosterCount(struct BattleStruct* battle);

// USA: func_02011544 / func_0201155c / func_02011570
void SetFlag_0x5ccc(struct BattleStruct* battle);
void ClearFlag_0x5ccc(struct BattleStruct* battle);
int GetFlag_0x5ccc(struct BattleStruct* battle);

// USA: func_02011584
void* GetWorldData(struct BattleStruct* battle);

// USA: func_02011590 / func_020115a8 / func_020115b4 / func_020115c0
bool HasValue_0x63d6(struct BattleStruct* battle);
unsigned char GetValue_0x63d6(struct BattleStruct* battle);
void SetValue_0x63d6(struct BattleStruct* battle, unsigned char value);
void ClearValue_0x63d6(struct BattleStruct* battle);

// USA: func_020115d0 / func_020115e8 / func_020115f4 / func_02011600
bool HasValue_0x63d8(struct BattleStruct* battle);
unsigned short GetValue_0x63d8(struct BattleStruct* battle);
void SetValue_0x63d8(struct BattleStruct* battle, unsigned short value);
void ClearValue_0x63d8(struct BattleStruct* battle);

// USA: func_02011610 / func_02011628 / func_02011634
bool HasValue_0x63da(struct BattleStruct* battle);
unsigned short GetValue_0x63da(struct BattleStruct* battle);
void ClearValue_0x63da(struct BattleStruct* battle);

}
