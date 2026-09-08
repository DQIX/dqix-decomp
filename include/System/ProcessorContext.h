#pragma once

#include "Timing.h"

struct ProcessorContext;
struct Mutex;

struct MutexList
{
    Mutex* pFirst;
    Mutex* pLast;
};

struct BlockedContextList
{
    ProcessorContext* first;
    ProcessorContext* last;

    // Insertions are sorted in priority order
    void Insert(ProcessorContext* context);
    ProcessorContext* PopFront();
    // Returns the context if it was found and NULL otherwise
    ProcessorContext* Remove(ProcessorContext* context);
};

int GenerateUniqueContextID();

struct ProcessorContext
{
    typedef void (*ExitRoutine)(int maybeExitCode);

    unsigned int programStatusRegister;
    unsigned int userModeRegisters[15];
    unsigned int resumeAddress;
    unsigned int supervisorStackPointer;

    struct MathRegisters
    {
        unsigned long long div_numer;
        unsigned long long div_denom;
        unsigned long long sqrt_param;
        unsigned short div_control;
        unsigned short sqrt_control;
    } mathRegisters;

    int blockState; // 0 = blocked, 1 = ready, 2 = inactive (newly created or destroyed)
    // All the active contexts are stored in a singly linked list
    ProcessorContext* pNext;
    unsigned int uniqueID;
    unsigned int priority; // lower values are prioritized
    unsigned int unknown_74;
    BlockedContextList* containerBlockedQueue;
    ProcessorContext* pPrevBlocked;
    ProcessorContext* pNextBlocked;
    Mutex* blockingMutex; // the mutex currently stopping this context from executing
    MutexList lockedMutexes; // all mutexes currently locked by this context
    unsigned int stackTop;
    unsigned int stackBottom;
    unsigned int stackUnknownTopSubspaceSize;
    BlockedContextList contextsAwaitingThisCompletion;
    unsigned int unknown_A4;
    unsigned int unknown_A8;
    unsigned int unknown_AC;
    Alarm* sleepAlarm;
    ExitRoutine exitProc; // seems to run when context execution is about to end / context is shut down
    unsigned int unknown_B8;
    unsigned int unknown_BC;
};

#define STACK_TOP_MAGIC 0x7bf9dd5b
#define STACK_BOTTOM_MAGIC 0xfddb597d
#define STACK_UNKNOWN_SECTION_MAGIC 0x597dfbd9

#define CONTEXT_STATE_BLOCKED 0
#define CONTEXT_STATE_READY 1
// newly created or completed
#define CONTEXT_STATE_INVALID 2

typedef void (*PFNSwitchContextProc)(ProcessorContext*, ProcessorContext*);

struct Struct_02111304
{
    unsigned short unknown_0;
    unsigned short unknown_2;
    ProcessorContext* activeContext;
    // first element of the global context list, sorted by priority
    ProcessorContext* firstContext;
    PFNSwitchContextProc switchContextProcB;
};

struct Struct_021112e0
{
    PFNSwitchContextProc switchContextProcA;
    unsigned int contextSwitchLock; // if nonzero, context switches will not occur. Acts like a refcount
    ProcessorContext** ppActiveContext; // points to 0x02111308, which holds a pointer to the active context
    int hasSetupPrimaryContext;
    unsigned int unknown_10;
    unsigned int unknown_14;
    unsigned int unknown_18;
    unsigned int unknown_1C;
    int contextUniqueIDCounter; // repeatedly incremented to generate unique ids
    Struct_02111304 substruct_24;
    ProcessorContext contextA; // alias for data_02111314
    ProcessorContext contextB; // alias for data_021113d4
};

#if defined(jpn)
#define data_021112e0 data_02110f80
#define data_02111304 data_02110fa4
#endif

extern Struct_021112e0 data_021112e0;
extern Struct_02111304 data_02111304;
extern ProcessorContext data_02111314;
extern ProcessorContext data_021113d4;

Mutex* PopFrontMutexFromList(MutexList* list);

ProcessorContext* InsertContextIntoGlobalList(ProcessorContext* context);
void RemoveContextFromGlobalList(ProcessorContext* context);

// switches to the first ready context (with the lowest priority value)
void SwitchContext();

int IsPrimaryContextSetUp();

void PopulateProcessorContext(ProcessorContext* context, unsigned int startAddress, unsigned int userdata,
    unsigned int stackBottom, unsigned int stackSize, unsigned int priority);

void ContextExecutionReturnProc();
void ExitContext(ProcessorContext* context, int exitCode);
void ExitCurrentContext(int code);
void ShutdownCurrentContext();
void ShutdownContext(ProcessorContext* context);
void CancelContextSleepAlarm(ProcessorContext* context);
void AwaitContextCompletion(ProcessorContext* context);

void BlockCurrentContext(BlockedContextList* blockQueue);
void UnblockContexts(BlockedContextList* blockQueue);
void MarkContextReadyAndSwitch(ProcessorContext* context);
ProcessorContext* GetFirstReadyContext();

// Wraps SwitchContext() in calls to disable then enable interrupts
// (every other use of SwitchContext() does this already)
void SwitchContextUninterrupted();

// Reorders the global context list to put the active one at the back of
// the sublist with equal priority. e.g. if the list is currently 
// high -> A -> B -> C -> low with A the currently active context, it will be
// shuffled to high -> B -> C -> A -> low.
void CycleCurrentPriorityContexts();

void MarkContextStackTopUnknownSubspace(ProcessorContext* context, unsigned int size);

bool ChangeContextPriority(ProcessorContext* context, unsigned int newPriority);
unsigned int GetContextPriority(ProcessorContext* context);

void SleepCurrentContext(unsigned int milliseconds);
void SleepCompletionProc(ProcessorContext** ppContext);

// Returns the old proc
PFNSwitchContextProc SetSwitchContextProcB(PFNSwitchContextProc proc);
// Used by one context to just repeatedly wait for interrupts
void InterruptWaitLoopFunction(void* unusedUserdata);

unsigned int AddContextSwitchLock();
unsigned int RemoveContextSwitchLock();

void SetContextEndProc(ProcessorContext* context, ProcessorContext::ExitRoutine proc);

void InitializeContextRegisters(ProcessorContext* context, unsigned int startAddress, unsigned int stackBottom);

// low level asm stuff, probably not going in decomp
extern "C" int SaveContext(ProcessorContext* context);
extern "C" void RestoreContext(ProcessorContext* context);

