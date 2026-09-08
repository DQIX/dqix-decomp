#include "Filesystem/CardReadManager.h"
#include "Filesystem/FSInnerDefs.h"
#include "System/Interrupts.h"
#include "System/GamecardBusOwnership.h"

#pragma optimize_for_size off

unsigned int SetReadContextPriority(unsigned int priority)
{
    CardReadManager* manager = (CardReadManager*)&data_021118e0;
    int priorState = DisableIRQInterrupts();
    unsigned int oldPrio = manager->contextPriority_108;
    manager->contextPriority_108 = priority;
    ChangeContextPriority(&manager->cartridgeReadContext, priority);
    SetIRQInterruptState(priorState);
    return oldPrio;
}

void NitroVM_Command_AcquireCardReadResources(unsigned short ownerID)
{
    LockCardReadManager(ownerID, 1);
    AcquireNDSBus(ownerID);
}

void NitroVM_Command_ReleaseCardReadResources(unsigned short ownerID)
{
    ReleaseNDSBus(ownerID);
    UnlockCardReadManager(ownerID, 1);
}