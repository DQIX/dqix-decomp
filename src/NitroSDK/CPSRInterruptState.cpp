#include "System/Interrupts.h"

// https://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
// Notably: 
// - 0x80 bit of cpsr set iff IRQ interrupts are disabled
// - 0x40 bit of cpsr set iff FIQ interrupts are disabled

#define BITMASK_IRQ_INTERRUPTS_DISABLED 0x80
#define BITMASK_FIQ_INTERRUPTS_DISABLED 0x40

#define BITMASK_ALL_INTERRUPTS_DISABLED (BITMASK_IRQ_INTERRUPTS_DISABLED | BITMASK_FIQ_INTERRUPTS_DISABLED)

#define BITMASK_PROCESSOR_MODE 0x1f

int EnableIRQInterrupts()
{
    int oldState;
    __asm("mrs oldState, cpsr");
    int newState = oldState & ~BITMASK_IRQ_INTERRUPTS_DISABLED;
    __asm("msr cpsr_c, newState");
    return oldState & BITMASK_IRQ_INTERRUPTS_DISABLED;
}

int DisableIRQInterrupts()
{
    int oldState;
    __asm("mrs oldState, cpsr");
    int newState = oldState | BITMASK_IRQ_INTERRUPTS_DISABLED;
    __asm("msr cpsr_c, newState");
    return oldState & BITMASK_IRQ_INTERRUPTS_DISABLED;
}

int SetIRQInterruptState(int to)
{
    int oldState;
    __asm("mrs oldState, cpsr");
    int newState = (oldState & ~BITMASK_IRQ_INTERRUPTS_DISABLED);
    newState |= to;
    __asm("msr cpsr_c, newState");
    return oldState & BITMASK_IRQ_INTERRUPTS_DISABLED;
}

int DisableIRQAndFIQInterrupts()
{
    int oldState;
    __asm("mrs oldState, cpsr");
    int newState = oldState | BITMASK_ALL_INTERRUPTS_DISABLED;
    __asm("msr cpsr_c, newState");
    return oldState & BITMASK_ALL_INTERRUPTS_DISABLED;
}

int SetIRQAndFIQInterruptState(int to)
{
    int oldState;
    __asm("mrs oldState, cpsr");
    int newState = (oldState & ~BITMASK_ALL_INTERRUPTS_DISABLED);
    newState |= to;
    __asm("msr cpsr_c, newState");
    return oldState & BITMASK_ALL_INTERRUPTS_DISABLED;
}

int GetIRQInterruptState()
{
    int state;
    __asm("mrs state, cpsr");
    return state & BITMASK_IRQ_INTERRUPTS_DISABLED;
}

int GetProcessorMode()
{
    int state;
    __asm("mrs state, cpsr");
    return state & BITMASK_PROCESSOR_MODE;
}