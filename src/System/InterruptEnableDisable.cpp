#include "System/Interrupts.h"

#define INTERRUPT_MASTER_ENABLE (*(volatile unsigned short*)0x04000208)
#define INTERRUPT_ENABLE (*(volatile unsigned int*)0x04000210)
#define INTERRUPT_REQUEST_FLAGS (*(volatile unsigned int*)0x04000214)

unsigned int SetSpecificInterruptsEnabled(unsigned int which)
{
    unsigned int oldIME = INTERRUPT_MASTER_ENABLE;
    INTERRUPT_MASTER_ENABLE = 0;

    unsigned int oldIE = INTERRUPT_ENABLE;
    INTERRUPT_ENABLE = which;

    // Why do we need this volatile read?
    (void)INTERRUPT_MASTER_ENABLE;

    INTERRUPT_MASTER_ENABLE = oldIME;
    
    return oldIE;
}

unsigned int EnableSpecificInterrupts(unsigned int mask)
{
    unsigned int oldIME = INTERRUPT_MASTER_ENABLE;
    INTERRUPT_MASTER_ENABLE = 0;

    unsigned int oldIE = INTERRUPT_ENABLE;
    INTERRUPT_ENABLE = oldIE | mask;

    // Why do we need this volatile read?
    (void)INTERRUPT_MASTER_ENABLE;

    INTERRUPT_MASTER_ENABLE = oldIME;
    
    return oldIE;
}

unsigned int DisableSpecificInterrupts(unsigned int mask)
{
    unsigned short oldIME = INTERRUPT_MASTER_ENABLE;
    INTERRUPT_MASTER_ENABLE = 0;

    unsigned int oldIE = INTERRUPT_ENABLE;
    INTERRUPT_ENABLE = oldIE & ~mask;

    // Why do we need this volatile read?
    (void)INTERRUPT_MASTER_ENABLE;

    INTERRUPT_MASTER_ENABLE = oldIME;
    return oldIE;
}

unsigned int AcknowledgeSpecificInterrupts(unsigned int flagMask)
{
    unsigned short oldIME = INTERRUPT_MASTER_ENABLE;
    INTERRUPT_MASTER_ENABLE = 0;

    unsigned int oldIF = INTERRUPT_REQUEST_FLAGS;
    INTERRUPT_REQUEST_FLAGS = flagMask;

    // Why do we need this volatile read?
    (void)INTERRUPT_MASTER_ENABLE;

    INTERRUPT_MASTER_ENABLE = oldIME;
    return oldIF;
}