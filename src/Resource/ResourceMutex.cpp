#include "Resource/ResourceMutex.h"

#ifdef jpn
#define func_020c745c func_020c8f28
#define func_020c75a4 func_020c9070
#endif

struct ResourceMutex
{
    unsigned char unknown_0;
    bool lockPrerequisiteA_;
    bool lockPrerequisiteB_;
    char padding_3;
    unsigned int lockCount_;
    Mutex mutex_;
};

static ResourceMutex s_resourceMutex;

extern "C"
{
    // set up the main context
    void func_020c745c();
    // check if the main context has been set up
    bool func_020c75a4();
}

void InitializeResourceMutex()
{
    if (!func_020c75a4())
        func_020c745c();

    ZeroInitializeMutex(&s_resourceMutex.mutex_);
    s_resourceMutex.lockPrerequisiteB_ = true;
    s_resourceMutex.unknown_0 = 1;
    s_resourceMutex.lockPrerequisiteA_ = true;
    s_resourceMutex.lockCount_ = 0;
}

bool SetResourceMutexOperational(bool to)
{
    bool oldStatus = s_resourceMutex.lockPrerequisiteA_;
    s_resourceMutex.lockPrerequisiteA_ = to;
    return oldStatus;
}

void LockResourceMutex()
{
    if (s_resourceMutex.lockPrerequisiteB_ && s_resourceMutex.lockPrerequisiteA_)
    {
        LockMutex(&s_resourceMutex.mutex_);
        s_resourceMutex.lockCount_++;
    }
}

void UnlockResourceMutex()
{
    if (s_resourceMutex.lockPrerequisiteB_ && s_resourceMutex.lockPrerequisiteA_)
    {
        s_resourceMutex.lockCount_--;
        UnlockMutex(&s_resourceMutex.mutex_);
    }
}

void SleepIfResourceMutexNotLocked(unsigned int milliseconds)
{
    if (s_resourceMutex.lockCount_ == 0)
        SleepCurrentContext(milliseconds);
}