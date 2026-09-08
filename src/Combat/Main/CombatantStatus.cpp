#include <globaldefs.h>
#include "Combat/Main/CombatantStatus.h"

// USA: func_02088840
ARM bool CanApplyStatus_0x40(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088854
ARM void ApplyStatus_0x40(struct CombatantStruct* combatant)
{
    combatant->statusDurations[0] = 4;
    combatant->statusCounters[0] = 0;
    combatant->statusFlags |= 0x40;
}

// USA: func_02088874
ARM void ClearStatus_0x40(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x40;
    combatant->statusDurations[0] = 0;
    combatant->statusCounters[0] = 0;
}

// USA: func_02088890
ARM bool CanApplyStatus_0x100(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_020888a4
ARM void ApplyStatus_0x100(struct CombatantStruct* combatant)
{
    combatant->statusDurations[1] = 6;
    combatant->statusCounters[1] = 0;
    combatant->statusFlags |= 0x100;
}

// USA: func_020888c4
ARM void ClearStatus_0x100(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x100;
    combatant->statusDurations[1] = 0;
    combatant->statusCounters[1] = 0;
}

// USA: func_020888e0
ARM bool CanApplyStatus_0x200(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_020888f4
ARM void ApplyStatus_0x200(struct CombatantStruct* combatant)
{
    combatant->statusDurations[2] = 5;
    combatant->statusCounters[2] = 0;
    combatant->statusFlags |= 0x200;
}

// USA: func_02088914
ARM void ClearStatus_0x200(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x200;
    combatant->statusDurations[2] = 0;
    combatant->statusCounters[2] = 0;
}

// USA: func_02088930
ARM bool CanApplyStatus_0x4000000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088944
ARM void ApplyStatus_0x4000000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[7] = 5;
    combatant->statusCounters[7] = 0;
    combatant->statusFlags |= 0x4000000;
}

// USA: func_02088964
ARM void ClearStatus_0x4000000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x4000000;
    combatant->statusDurations[7] = 0;
    combatant->statusCounters[7] = 0;
}

// USA: func_02088980
ARM bool CanApplyStatus_0x8000000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088994
ARM void ApplyStatus_0x8000000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[3] = 5;
    combatant->statusCounters[3] = 0;
    combatant->statusFlags |= 0x8000000;
}

// USA: func_020889b4
ARM void ClearStatus_0x8000000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x8000000;
    combatant->statusDurations[3] = 0;
    combatant->statusCounters[3] = 0;
}

// USA: func_020889d0
ARM bool CanApplyStatus_0x10000000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_020889e4
ARM void ApplyStatus_0x10000000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[4] = 4;
    combatant->statusCounters[4] = 0;
    combatant->statusFlags |= 0x10000000;
}

// USA: func_02088a04
ARM void ClearStatus_0x10000000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x10000000;
    combatant->statusDurations[4] = 0;
    combatant->statusCounters[4] = 0;
}

// USA: func_02088a20
ARM bool CanApplyStatus_0x20000000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088a34
ARM void ApplyStatus_0x20000000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[5] = 4;
    combatant->statusCounters[5] = 0;
    combatant->statusFlags |= 0x20000000;
}

// USA: func_02088a54
ARM void ClearStatus_0x20000000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x20000000;
    combatant->statusDurations[5] = 0;
    combatant->statusCounters[5] = 0;
}

// USA: func_02088a70
ARM bool CanEnterState_0x100000(struct CombatantStruct* combatant)
{
    if (combatant->statusFlags & 1)
        return false;

    return (combatant->stateFlags & 0x40) == 0;
}

// USA: func_02088a94
ARM void EnterState_0x100000(struct CombatantStruct* combatant)
{
    combatant->statusFlags = (combatant->statusFlags | 0x100000) & ~0x200000;
}

// USA: func_02088aa8
ARM void LeaveState_0x100000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x100000;
}

// USA: func_02088ab8
ARM bool CanEnterState_0x200000(struct CombatantStruct* combatant)
{
    if (combatant->statusFlags & 1)
        return false;

    return (combatant->stateFlags & 0x40) == 0;
}

// USA: func_02088adc
ARM void EnterState_0x200000(struct CombatantStruct* combatant)
{
    combatant->statusFlags = (combatant->statusFlags | 0x200000) & ~0x100000;
}

// USA: func_02088af0
ARM void LeaveState_0x200000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x200000;
}

// USA: func_02088b00
ARM bool CanApplyStatus_0x400000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088b14
ARM void ApplyStatus_0x400000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[8] = 6;
    combatant->statusCounters[8] = 0;
    combatant->statusFlags |= 0x400000;
}

// USA: func_02088b34
ARM void ClearStatus_0x400000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x400000;
    combatant->statusDurations[8] = 0;
    combatant->statusCounters[8] = 0;
}

// USA: func_02088b50
ARM bool CanApplyStatus_0x40000000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088b64
ARM void ApplyStatus_0x40000000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[9] = 6;
    combatant->statusCounters[9] = 0;
    combatant->statusFlags |= 0x40000000;
}

// USA: func_02088b84
ARM void ClearStatus_0x40000000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x40000000;
    combatant->statusDurations[9] = 0;
    combatant->statusCounters[9] = 0;
}

// USA: func_02088ba0
ARM bool CanApplyStatus_0x80000000(struct CombatantStruct* combatant)
{
    return (combatant->statusFlags & 1) == 0;
}

// USA: func_02088bb4
ARM void ApplyStatus_0x80000000(struct CombatantStruct* combatant)
{
    combatant->statusDurations[10] = 6;
    combatant->statusCounters[10] = 0;
    combatant->statusFlags |= 0x80000000;
}

// USA: func_02088bd4
ARM void ClearStatus_0x80000000(struct CombatantStruct* combatant)
{
    combatant->statusFlags &= ~0x80000000;
    combatant->statusDurations[10] = 0;
    combatant->statusCounters[10] = 0;
}
