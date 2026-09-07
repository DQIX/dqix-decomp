#include "System/Mutex.h"
#include "System/Interrupts.h"

#pragma optimize_for_size off

extern "C"
{
}
void AddMutexToContextLockedList(ProcessorContext* context, Mutex* mutex);
void RemoveMutexFromContextLockedList(ProcessorContext* context, Mutex* mutex);

void ZeroInitializeMutex(Mutex* mutex)
{
    mutex->waitingContexts_.first = mutex->waitingContexts_.last = NULL;
    mutex->ownedContext_ = NULL;
    mutex->ownerRefcount_ = 0;
}

void LockMutex(Mutex* mutex)
{
    int priorState = DisableIRQInterrupts();
    ProcessorContext* thisContext = data_02111304.activeContext;
    while (true)
    {
        if (mutex->ownedContext_ == NULL)
        {
            mutex->ownedContext_ = thisContext;
            mutex->ownerRefcount_++;
            AddMutexToContextLockedList(thisContext, mutex);
            break;
        }
        else if (mutex->ownedContext_ == thisContext)
        {
            mutex->ownerRefcount_++;
            break;
        }
        else
        {
            thisContext->blockingMutex = mutex;
            BlockCurrentContext(&mutex->waitingContexts_);
            thisContext->blockingMutex = NULL;
        }
    }
    SetIRQInterruptState(priorState);
}

void UnlockMutex(Mutex* mutex)
{
    int priorState = DisableIRQInterrupts();
    ProcessorContext* thisContext = data_02111304.activeContext;
    if (mutex->ownedContext_ == thisContext)
    {
        mutex->ownerRefcount_--;
        if (mutex->ownerRefcount_ == 0)
        {
            RemoveMutexFromContextLockedList(thisContext, mutex);
            mutex->ownedContext_ = NULL;
            UnblockContexts(&mutex->waitingContexts_);
        }
    }
    SetIRQInterruptState(priorState);
}

void UnlockAllMutexesLockedByContext(ProcessorContext *context)
{
    if (context->lockedMutexes.pFirst != NULL)
    {
        do
        {
            Mutex* mutex = PopFrontMutexFromList(&context->lockedMutexes);
            mutex->ownerRefcount_ = 0;
            mutex->ownedContext_ = NULL;
            UnblockContexts(&mutex->waitingContexts_);
        } while (context->lockedMutexes.pFirst != NULL);
    }
}

bool TryLockMutex(Mutex* mutex)
{
    int priorState = DisableIRQInterrupts();
    ProcessorContext* thisContext = data_02111304.activeContext;
    bool success;
    if (mutex->ownedContext_ == NULL)
    {
        mutex->ownedContext_ = thisContext;
        mutex->ownerRefcount_++;
        AddMutexToContextLockedList(thisContext, mutex);
        success = true;
    }
    else if (mutex->ownedContext_ == thisContext)
    {
        mutex->ownerRefcount_++;
        success = true;
    }
    else
        success = false;

    SetIRQInterruptState(priorState);
    return success;
}

// usa: func_020c8204
// can be made static later
void AddMutexToContextLockedList(ProcessorContext* context, Mutex* mutex)
{
    Mutex* oldLast = context->lockedMutexes.pLast;

    if (oldLast == NULL)
        context->lockedMutexes.pFirst = mutex;
    else
        oldLast->pNext_ = mutex;

    mutex->pPrev_ = oldLast;
    mutex->pNext_ = NULL;
    context->lockedMutexes.pLast = mutex;
}

// usa: func_020c8228
// can be made static later
void RemoveMutexFromContextLockedList(ProcessorContext* context, Mutex* mutex)
{
    Mutex* after = mutex->pNext_;
    Mutex* before = mutex->pPrev_;

    if (after == NULL)
        context->lockedMutexes.pLast = before;
    else
        after->pPrev_ = before;

    if (before == NULL)
        context->lockedMutexes.pFirst = after;
    else
        before->pNext_ = after;
}