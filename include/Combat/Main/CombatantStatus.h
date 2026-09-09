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

// USA: func_02088a70 / func_02088a94 / func_02088aa8
// Setting this state clears 0x200000; the two are mutually exclusive.
bool CanEnterState_0x100000(struct CombatantStruct* combatant);
void EnterState_0x100000(struct CombatantStruct* combatant);
void LeaveState_0x100000(struct CombatantStruct* combatant);

// USA: func_02088ab8 / func_02088adc / func_02088af0
// Setting this state clears 0x100000; the two are mutually exclusive.
bool CanEnterState_0x200000(struct CombatantStruct* combatant);
void EnterState_0x200000(struct CombatantStruct* combatant);
void LeaveState_0x200000(struct CombatantStruct* combatant);

// USA: func_02088b00 / func_02088b14 / func_02088b34
bool CanApplyStatus_0x400000(struct CombatantStruct* combatant);
void ApplyStatus_0x400000(struct CombatantStruct* combatant);
void ClearStatus_0x400000(struct CombatantStruct* combatant);

// USA: func_02088b50 / func_02088b64 / func_02088b84
bool CanApplyStatus_0x40000000(struct CombatantStruct* combatant);
void ApplyStatus_0x40000000(struct CombatantStruct* combatant);
void ClearStatus_0x40000000(struct CombatantStruct* combatant);

// USA: func_02088ba0 / func_02088bb4 / func_02088bd4
bool CanApplyStatus_0x80000000(struct CombatantStruct* combatant);
void ApplyStatus_0x80000000(struct CombatantStruct* combatant);
void ClearStatus_0x80000000(struct CombatantStruct* combatant);

// USA: func_02088bf0 / func_02088c10 / func_02088c38
// Applying this tail-calls ClearState_0x4: the two are exclusive.
bool CanApplyState_0x2(struct CombatantStruct* combatant);
void ApplyState_0x2(struct CombatantStruct* combatant);
void ClearState_0x2(struct CombatantStruct* combatant);

// USA: func_02088c54 / func_02088c74 / func_02088c9c
// Applying this tail-calls ClearState_0x2: the two are exclusive.
bool CanApplyState_0x4(struct CombatantStruct* combatant);
void ApplyState_0x4(struct CombatantStruct* combatant);
void ClearState_0x4(struct CombatantStruct* combatant);

// USA: func_02088cb8 / func_02088ccc / func_02088cf4
// Applying this tail-calls ClearState_0x10: the two are exclusive.
bool CanApplyState_0x8(struct CombatantStruct* combatant);
void ApplyState_0x8(struct CombatantStruct* combatant);
void ClearState_0x8(struct CombatantStruct* combatant);

// USA: func_02088d10 / func_02088d24 / func_02088d4c
// Applying this tail-calls ClearState_0x8: the two are exclusive.
bool CanApplyState_0x10(struct CombatantStruct* combatant);
void ApplyState_0x10(struct CombatantStruct* combatant);
void ClearState_0x10(struct CombatantStruct* combatant);

// USA: func_02088d68 / func_02088d7c / func_02088d9c
bool CanApplyState_0x100(struct CombatantStruct* combatant);
void ApplyState_0x100(struct CombatantStruct* combatant);
void ClearState_0x100(struct CombatantStruct* combatant);

// USA: func_02088db8 / func_02088dc8
void SetState_0x20(struct CombatantStruct* combatant);
void ClearState_0x20(struct CombatantStruct* combatant);

}
