#include "System/Interrupts.h"
#include "System/DMA.h"
#include "System/BiosData.h"
#include "System/DTCM.h"
#include <globaldefs.h>
#include <asmhacks.h>

#pragma optimize_for_size off

#if defined(jpn)
#define data_0211127c data_02110f1c
#define data_020f2274 data_020f23e0
#endif

struct DMAOrTimerResponse
{
    DMACompletionCallback callback;
    unsigned int stayEnabledAfter;
    int userdata;
};

// 0-3 are DMA, 4-7 are timers
extern DMAOrTimerResponse data_0211127c[8];
// maps index in the previous array to interrupt ID
extern unsigned short data_020f2274[8];

inline DMACompletionCallback& CallbackByIndex(int n, int base = 0)
{
    return *(DMACompletionCallback*)((unsigned int)&data_0211127c[base].callback + n * sizeof(DMAOrTimerResponse));
}

inline unsigned int& ShouldStayEnabledByIndex(int n, int base = 0)
{
    return *(unsigned int*)((unsigned int)&data_0211127c[base].stayEnabledAfter + n * sizeof(DMAOrTimerResponse));
}

inline int& CallbackUserdataByIndex(int n, int base = 0)
{
    return *(int*)((unsigned int)&data_0211127c[base].userdata + n * sizeof(DMAOrTimerResponse));
}

// Start of exposed functions

void WaitForInterrupt(bool onlySubsequent, unsigned int mask)
{
    int priorState = DisableIRQInterrupts();
    if (onlySubsequent)
        DTCM_DATA.interruptsFired &= ~mask;
    SetIRQInterruptState(priorState);
    
    if (!(mask & DTCM_DATA.interruptsFired))
    {
        BlockedContextList* list = &data_027e0000.block_60;
        unsigned int* pData;
        do {
            pData = &DTCM_DATA.interruptsFired;
            BlockCurrentContext(list);
        } while (!(mask & *pData));
    }
    DECLARE_ASM_NOP();
}

void EmptyInterruptHandler() {}

// This function matches with wrong registers
void OnDMAOrTimerCompletion(int index)
{
    unsigned int irqId = data_020f2274[index];
    unsigned int irqMask = 1 << irqId;

    DMACompletionCallback callback = CallbackByIndex(index);
    CallbackByIndex(index) = NULL;

    if (callback != NULL)
        callback(CallbackUserdataByIndex(index));
        
    
    unsigned int stayEnabled = 0;
    
    DTCMData& itcm = DTCM_DATA;
    stayEnabled = ShouldStayEnabledByIndex(index);
    itcm.interruptsFired |= irqMask;
    

    if (!stayEnabled)
    {
        DisableSpecificInterrupts(irqMask);
    }
}

void DMA0InterruptHandler() { OnDMAOrTimerCompletion(0); }
void DMA1InterruptHandler() { OnDMAOrTimerCompletion(1); }
void DMA2InterruptHandler() { OnDMAOrTimerCompletion(2); }
void DMA3InterruptHandler() { OnDMAOrTimerCompletion(3); }

void Timer0OverflowInterruptHandler() { OnDMAOrTimerCompletion(4); }
void Timer1OverflowInterruptHandler() { OnDMAOrTimerCompletion(5); }
void Timer2OverflowInterruptHandler() { OnDMAOrTimerCompletion(6); }
void Timer3OverflowInterruptHandler() { OnDMAOrTimerCompletion(7); }

void InitializeInterruptContextBlock_020c6ad4()
{
    BlockedContextList& list = GetInterruptDataBlockedContextList();
    list.first = list.last = NULL;
}

// This function matches but with wrong registers
// proc can either be of type void(*)() for regular interrupts,
// or void(*)(int) for DMA / timer response interrupts
void SetInterruptHandler(unsigned int mask, const void* proc)
{
    InterruptHandlerProc* regularTable;
    int dmaTimerIndex;
    DMAOrTimerResponse* specialTable;
    
    int interruptID;
    
    regularTable = data_027e0000.interruptProcTable;
    specialTable = data_0211127c;
    interruptID = 0;
    do {
        
        if (mask & 1)
        {
            DMAOrTimerResponse* dmaTimerData = NULL;
            // DMA channels
            if (interruptID >= 8 && interruptID <= 11)
            {
                int dmaTimerIndex = interruptID - 8;
                dmaTimerData = &specialTable[dmaTimerIndex];
            }
            // timer overflows
            else if (interruptID >= 3 && interruptID <= 6)
            {
                dmaTimerIndex = interruptID + 1;
                dmaTimerData = &specialTable[dmaTimerIndex];
            }
            else
            {
                regularTable[interruptID] = (InterruptHandlerProc)proc;
            }

            if (dmaTimerData != NULL)
            {
                dmaTimerData->callback = (DMACompletionCallback)proc;
                dmaTimerData->userdata = 0;
                dmaTimerData->stayEnabledAfter = true;
            }
        }
        interruptID++;
        mask >>= 1;
    } while (interruptID < 22);
}

InterruptHandlerProc GetInterruptHandler(unsigned int mask)
{
    int interruptId = 0;
    InterruptHandlerProc* pProc = &data_027e0000.interruptProcTable[0];
    do
    {
        if (!(mask & 1))
            continue;

        if (interruptId >= 8 && interruptId <= 11)
        {
            return (InterruptHandlerProc)data_0211127c[interruptId - 8].callback;
        }
        else if (interruptId >= 3 && interruptId <= 6)
        {
            return (InterruptHandlerProc)data_0211127c[interruptId + 1].callback;
        }
        else
        {
            return *pProc;
        }

    } while (interruptId++, mask >>= 1, pProc++, interruptId < 22);
    return NULL;
}

void SetDMACompletionCallback(int channel, DMACompletionCallback callback, int userdata)
{
    CallbackByIndex(channel) = callback;
    CallbackUserdataByIndex(channel) = userdata;
    unsigned int prior = EnableSpecificInterrupts(1 << (channel + 8));
    ShouldStayEnabledByIndex(channel) = prior & (1 << (channel + 8));
}

void SetTimerOverflowCallback(int timer, DMACompletionCallback callback, int userdata)
{
    CallbackByIndex(timer, 4) = callback;
    CallbackUserdataByIndex(timer, 4) = userdata;
    EnableSpecificInterrupts(1 << (timer + 3));
    ShouldStayEnabledByIndex(timer, 4) = true;
}