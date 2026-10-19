#pragma once

#include "std_library_functions.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_02111634 data_021112d4
#define data_02111638 data_021112d8
#define data_02111644 data_021112e4
#define data_02111648 data_021112e8
#endif

struct ProcessorContext;

// sizeof == 44 == 0x2C
struct Alarm
{
    // parameter might be a generic userdata pointer
    typedef void (*PFNCompletion)(ProcessorContext**);

    PFNCompletion completionProc;
    ProcessorContext** ppContext;
    unsigned int unknown_8;
    uint64_t alarmTime; // timestamp at which this alarm 'rings' i.e. the completion proc runs
    Alarm* pPrev;
    Alarm* pNext;
    uint64_t alarmIntervalLength;
    uint64_t alarmIntervalResidue; // if e.g. length = 5 and residue = 2, then the 
                                   // alarm sounds when global tick counter = 2, 7, 12, 17, ...
};

extern unsigned short data_02111634;

struct Timer64Bit
{
    unsigned short isInitialized;
    int reloadTimerOnNextInterrupt;
    volatile uint64_t numTimerOverflows;
};

extern Timer64Bit data_02111638;

struct ActiveAlarmList
{
    unsigned short isInitialized; 
    Alarm* pFirstAlarm;
    Alarm* pLastAlarm;
};

extern ActiveAlarmList data_02111648;

#define TIMER_TICKS_PER_MILLISECOND 33514 // approximate, should be 33513.982

void MarkAlarmInitializationFlagBit(int);

void Initialize64BitTimer();
int Is64BitTimerInitialized();
void On16BitTimerOverflow(int);

uint64_t GetCurrentTimestamp();
unsigned short GetMain16BitTimerCounter();

void MarkNextAlarmToSound(Alarm* timing);

void InitializeActiveAlarmList();
int IsAlarmListInitialized();
void ZeroInitializeAlarm(Alarm* alarm);
// Registers for a single activation, so interval alarms will have to repeatedly
// call this (the interrupt handler sorts this out). ringTime is ignored for
// interval alarms.
void RegisterAlarm(Alarm* alarm, uint64_t ringTime);

void SetTimeout(Alarm* alarm, uint64_t delay, Alarm::PFNCompletion callback, ProcessorContext** ppContext);
void SetInterval(Alarm* alarm, uint64_t residue, uint64_t interval,
    Alarm::PFNCompletion callback, ProcessorContext** ppContext);
void CancelAlarm(Alarm* alarm);

void Timer1OverflowInterruptRoutine(int); // not sure if void() or void(int) or int(int)
void HandleTimer1Overflow();