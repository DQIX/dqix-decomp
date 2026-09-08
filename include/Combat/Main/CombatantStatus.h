#pragma once

#include "Combat/Main/BattleList.h"

// Seven status effects, each with a test, an apply and a clear, contiguous at
// 0x02088840..0x02088a70 and called from overlay 24. Names carry the status
// bit because none of the seven is identified.
//
// The seven test functions are byte-identical: every one reads statusFlags and
// returns whether bit 0 is clear, not its own bit. So the test is a shared
// precondition on the combatant, not a query about the status.

extern "C"
{

// USA: func_02088840 / func_02088854 / func_02088874
bool CanApplyStatus_0x40(struct CombatantStruct* combatant);
void ApplyStatus_0x40(struct CombatantStruct* combatant);
void ClearStatus_0x40(struct CombatantStruct* combatant);

// USA: func_02088890 / func_020888a4 / func_020888c4
bool CanApplyStatus_0x100(struct CombatantStruct* combatant);
void ApplyStatus_0x100(struct CombatantStruct* combatant);
void ClearStatus_0x100(struct CombatantStruct* combatant);

// USA: func_020888e0 / func_020888f4 / func_02088914
bool CanApplyStatus_0x200(struct CombatantStruct* combatant);
void ApplyStatus_0x200(struct CombatantStruct* combatant);
void ClearStatus_0x200(struct CombatantStruct* combatant);

// USA: func_02088930 / func_02088944 / func_02088964
bool CanApplyStatus_0x4000000(struct CombatantStruct* combatant);
void ApplyStatus_0x4000000(struct CombatantStruct* combatant);
void ClearStatus_0x4000000(struct CombatantStruct* combatant);

// USA: func_02088980 / func_02088994 / func_020889b4
bool CanApplyStatus_0x8000000(struct CombatantStruct* combatant);
void ApplyStatus_0x8000000(struct CombatantStruct* combatant);
void ClearStatus_0x8000000(struct CombatantStruct* combatant);

// USA: func_020889d0 / func_020889e4 / func_02088a04
bool CanApplyStatus_0x10000000(struct CombatantStruct* combatant);
void ApplyStatus_0x10000000(struct CombatantStruct* combatant);
void ClearStatus_0x10000000(struct CombatantStruct* combatant);

// USA: func_02088a20 / func_02088a34 / func_02088a54
bool CanApplyStatus_0x20000000(struct CombatantStruct* combatant);
void ApplyStatus_0x20000000(struct CombatantStruct* combatant);
void ClearStatus_0x20000000(struct CombatantStruct* combatant);

}
