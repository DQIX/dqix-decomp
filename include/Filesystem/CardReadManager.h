#pragma once

#include "System/ProcessorContext.h"
#include "LowNitroHandle.h"
#include "NitroVM.h"

// sizeof == 0x60
struct Arm7CardReadData
{
    int unknown_0;
    int unknown_4[23];
};

// sizeof <= 0x620
#define CARTRIDGE_READ_CONTEXT_FLAG_0 0
#define READ_MANAGER_FLAG_HARDWARE_READ_IN_PROGRESS 2
#define READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING 3
#define CARTRIDGE_READ_CONTEXT_FLAG_4 4
#define READ_MANAGER_FLAG_AWAITING_ARM7_ACTION 5
#define CARTRIDGE_READ_CONTEXT_FLAG_6 6

struct CardReadManager
{
    typedef void (*ReadProc)(CardReadManager*);
    typedef void (*CompletionCallback)(NitroHandle*);

    Arm7CardReadData* pSharedData;
    int unknown_4;
    struct CardReadManagerLock {
        // two of {owner, multiplicity, taskType} should probably be volatile,
        // but not all three. see the lock & initialize functions
        volatile int owner;
        int multiplicity;
        BlockedContextList waitingContexts;
        int taskType;
    } lock;
    unsigned int cartridgeReadOffset;
    unsigned char* writeDst;
    unsigned int writeLength;
    unsigned int dmaChannel;
    int unknown_2c[3];
    CompletionCallback onComplete;
    NitroHandle* handle;
    ReadProc cartridgeReadProc;
    // cartridgeReadContext runs a loop that handles incoming tasks, but
    // if you request a synchronous task then execution can occur on a different
    // context in the meantime
    ProcessorContext cartridgeReadContext;
    ProcessorContext* currentTaskExecutionContext;
    // Set to 4 and 8 in different places
    unsigned int contextPriority_108;
    BlockedContextList ongoingReadBlock; 
    volatile unsigned int flags;
    // if you need to clean more than this amount of the cache, then just
    // clean the whole thing instead
    unsigned int instructionCacheCleanThreshold;
    unsigned int dataCacheCleanThreshold;

    // Start of this seems to have game title, but can't find where it gets
    // copied in
    char unknown[0x100];
    char readContextStack[0x400];
};

void SendTaskToReadContext(CardReadManager::ReadProc task);

void LockCardReadManager(unsigned short ownerID, int taskType);
void UnlockCardReadManager(unsigned short ownerID, int taskType);

void InitializeCardReadManager();
int IsCardReadManagerInitialized();
void VerifyCardReadManagerInitialized();

bool AwaitCardReadManagerIdle();
bool IsCardReadManagerIdle();
int GetCardReadManagerSharedStatus();

unsigned int SetReadContextPriority(unsigned int priority);
void NitroVM_Command_AcquireCardReadResources(unsigned short ownerID);
void NitroVM_Command_ReleaseCardReadResources(unsigned short ownerID);

#if defined(jpn)
#define func_020d0f28 func_020d29f4
#endif

void SendGamecardBusCommand(unsigned int firstWord, unsigned int secondWord);
unsigned int SetupNormalGamecardBusCommandMode();
// note: even if set to async, you'll still have to wait for any ongoing operation
// to finish before this one can be dispatched
void LoadDataFromCartridgeToMemory(unsigned int dmaChannel,
    unsigned int cartridgeOffset, void* dest, unsigned int length,
    CardReadManager::CompletionCallback onComplete, NitroHandle* handle, CBool async);

void InitializeCardReading();

void IPCCommand11Proc(unsigned int command, unsigned int argument, unsigned int flag);
void CartridgeReadContextLoop();