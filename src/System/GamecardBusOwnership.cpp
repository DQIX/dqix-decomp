#include "System/GamecardBusOwnership.h"
#include "System/Interrupts.h"
#include <globaldefs.h>
#include <asmhacks.h>
#include "std_library_functions.h"

extern int data_021112dc;

#define REG_EXTMEMCTRL (*(volatile unsigned short*)0x04000204)
#define EXTMEMCTRL_FLAG_RELINQUISH_GBA_BUS (1 << 7)
#define EXTMEMCTRL_FLAG_RELINQUISH_NDS_BUS (1 << 11)

#define ADDR_REGISTERED_OWNERS_LOW 0x027fffb0
#define REGISTERED_OWNER_FLAGS ((unsigned int*)ADDR_REGISTERED_OWNERS_LOW)


#define PTR_NDS_BUS_LOCK ((GamecardBusLock*)0x027fffe0)
#define PTR_GBA_BUS_LOCK ((GamecardBusLock*)0x027fffe8)
#define PTR_UNKNOWN_BUS_LOCK ((GamecardBusLock*)0x027ffff0)

#pragma optimize_for_size off

#if defined(jpn)
#define func_020ca7e0 func_020cc2ac
#endif

extern "C"
{
    void WaitByLoop(int);

    // aligned memset clone
    void func_020ca3ec(int val, void* dst, unsigned len);

    // Performs an atomic swap
    unsigned int func_020ca7e0(unsigned int newValue, volatile unsigned int* atomic);
}

int TryLockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onLock)(), bool strict);
int WeakLockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onLock)());
int WeakUnlockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onUnlock)());

void MarkGBABusAcquired(); // acquire gba bus
void MarkGBABusReleased(); // release gba bus

void MarkNDSBusAcquired();
void MarkNDSBusReleased();

#if false
// this is almost correct except the first bit, excluding it for now
void InitializeGamecardBusOwnership()
{
    if (data_021112dc)
    {
        return;
    }
    
    GamecardBusLock* ndsLock = PTR_UNKNOWN_BUS_LOCK;
    data_021112dc = true;
    ndsLock->atomic = 0;
    
    WeakLockGamecardBusLock(126, ndsLock, NULL);

    if (ndsLock->unknown_6)
    {
        do
        {
            WaitByLoop(0x400);
        } while (ndsLock->unknown_6);
    }

    REGISTERED_OWNER_FLAGS[0] = 0xffffffff;
    REGISTERED_OWNER_FLAGS[1] = 0xffff0000;

    func_020ca3ec(0, (void*)0x027fffc0, 0x28);
    REG_EXTMEMCTRL |= EXTMEMCTRL_FLAG_RELINQUISH_NDS_BUS;
    REG_EXTMEMCTRL |= EXTMEMCTRL_FLAG_RELINQUISH_GBA_BUS;

    WeakUnlockGamecardBusLock(126, ndsLock, NULL);
    WeakLockGamecardBusLock(127, ndsLock, NULL);
}
#endif

// can be static
int LockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onLock)(), bool strict)
{
    if (TryLockGamecardBusLock(owner, lock, onLock, strict) > 0)
    {
        do {
            WaitByLoop(0x400);
        } while (TryLockGamecardBusLock(owner, lock, onLock, strict) > 0);
    }
}

// can be static
int WeakLockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onLock)())
{
    return LockGamecardBusLock(owner, lock, onLock, false);
}

// can be static
int UnlockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onUnlock)(), bool strict)
{
    if (owner != lock->ownerID)
        return -2;
    
    int priorState;
    if (strict)
        priorState = DisableIRQAndFIQInterrupts();
    else
        priorState = DisableIRQInterrupts();

    lock->ownerID = 0;
    if (onUnlock != NULL)
        onUnlock();

    lock->atomic = 0;

    if (strict)
        SetIRQAndFIQInterruptState(priorState);
    else
        SetIRQInterruptState(priorState);
    return 0;
}

// can be static
int WeakUnlockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onUnlock)())
{
    return UnlockGamecardBusLock(owner, lock, onUnlock, false);
}

// can be static
int TryLockGamecardBusLock(unsigned short owner, GamecardBusLock* lock, void (*onLock)(), bool strict)
{
    int priorState;
    if (strict)
        priorState = DisableIRQAndFIQInterrupts();
    else
        priorState = DisableIRQInterrupts();

    int oldAtomic = func_020ca7e0(owner, &lock->atomic);
    if (oldAtomic == 0)
    {
        if (onLock != NULL)
            onLock();
        lock->ownerID = owner;
    }

    if (strict)
        SetIRQAndFIQInterruptState(priorState);
    else
        SetIRQInterruptState(priorState);
    return oldAtomic;
}

// can be static
int InternalReleaseGBABus(unsigned short owner)
{
    return UnlockGamecardBusLock(owner, PTR_GBA_BUS_LOCK, &MarkGBABusReleased, true);
}

// must be exposed
#ifdef __MWERKS__
asm int ReleaseGBABus(unsigned short owner)
{
    ldr r1, =InternalReleaseGBABus
    bx r1
}
#else
int ReleaseGBABus(unsigned short owner)
{
    return InternalReleaseGBABus(owner);
}
#endif

// must be exposed
int TryAcquireGBABus(unsigned short owner)
{
    return TryLockGamecardBusLock(owner, PTR_GBA_BUS_LOCK, &MarkGBABusAcquired, true);
}

// can be static
void MarkGBABusAcquired()
{
    REG_EXTMEMCTRL &= ~EXTMEMCTRL_FLAG_RELINQUISH_GBA_BUS;
}

// can be static
void MarkGBABusReleased()
{
    REG_EXTMEMCTRL |= EXTMEMCTRL_FLAG_RELINQUISH_GBA_BUS;
}

// must be exposed
int AcquireNDSBus(unsigned short owner)
{
    return WeakLockGamecardBusLock(owner, PTR_NDS_BUS_LOCK, &MarkNDSBusAcquired);
}

// must be exposed
int ReleaseNDSBus(unsigned short owner)
{
    return WeakUnlockGamecardBusLock(owner, PTR_NDS_BUS_LOCK, &MarkNDSBusReleased);
}

// can be static
void MarkNDSBusAcquired()
{
    REG_EXTMEMCTRL &= ~EXTMEMCTRL_FLAG_RELINQUISH_NDS_BUS;
}

// can be static
void MarkNDSBusReleased()
{
    REG_EXTMEMCTRL |= EXTMEMCTRL_FLAG_RELINQUISH_NDS_BUS;
}

// must be exposed
unsigned short GetLockOwner(GamecardBusLock* lock)
{
    return lock->ownerID;
}

inline int leadZeroCount(unsigned int what)
{
    int ret;
    __asm("clz %[output], %[input]" : : [output] "=r" (ret), [input] "r" (what));
    return ret;
}

// GenerateLockOwnerID and ReleaseLockOwnerID are the next two functions,
// but they appear to be written in assembly (they use clz which the compiler
// never emits, and more generally are quite weird for compiled code)