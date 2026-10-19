#pragma once

#include "ProcessorContext.h"

// This only works for arm9-exclusive objects. The mutex functions
// work by disabling IRQ interrupts and manually yielding the thread
struct Mutex
{
    BlockedContextList waitingContexts_;
    ProcessorContext* ownedContext_;
    // you can lock the mutex multiple times on the same thread
    unsigned int ownerRefcount_;
    Mutex* pNext_;
    Mutex* pPrev_;
};

// usa: func_020c805c
void ZeroInitializeMutex(Mutex* mutex);
// usa: func_020c8074
void LockMutex(Mutex* mutex);
// usa: func_020c80f8
void UnlockMutex(Mutex* mutex);
// usa: func_020c8154
void UnlockAllMutexesLockedByContext(ProcessorContext* context);
// usa: func_020c8190
bool TryLockMutex(Mutex* mutex);