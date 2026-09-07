#pragma once

#include "../System/ProcessorContext.h"
#include "../System/Mutex.h"

// Provides a global instance of a mutex to be locked/unlocked by
// various different resource-related operations. For example, it's used
// by memory allocators and the BackgroundLoader for files

// usa: func_020d96b0
void InitializeResourceMutex();

// usa: func_020d96f4
// If set to false, lock and unlock operations on the mutex will not
// go through. If set to true, they will behave as normal.
// Returns the old status.
bool SetResourceMutexOperational(bool to);
// usa: func_020d970c
void LockResourceMutex();
// usa: func_020d974c
void UnlockResourceMutex();
// usa: func_020d9788
void SleepIfResourceMutexNotLocked(unsigned int milliseconds);