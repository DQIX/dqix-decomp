#pragma once

// Implentations of these functions are spread between
// InterruptEnableDisable.cpp
// InterruptHandler.cpp
// CPSRInterruptState.cpp

#include "DMA.h"

typedef void (*InterruptHandlerProc)();

void WaitForInterrupt(bool onlySubsequent, unsigned int mask);
void OnDMAOrTimerCompletion(int index);
void SetInterruptHandler(unsigned int mask, const void* proc);
InterruptHandlerProc GetInterruptHandler(unsigned int mask);
void SetDMACompletionCallback(int channel, DMACompletionCallback callback, int userdata);
void SetTimerOverflowCallback(int timer, DMACompletionCallback callback, int userdata);

/*
copied from GBATek documentation:
  0     LCD V-Blank
  1     LCD H-Blank
  2     LCD V-Counter Match
  3     Timer 0 Overflow
  4     Timer 1 Overflow
  5     Timer 2 Overflow
  6     Timer 3 Overflow
  7     NDS7 only: SIO/RCNT/RTC (Real Time Clock)
  8     DMA 0
  9     DMA 1
  10    DMA 2
  11    DMA 3
  12    Keypad
  13    GBA-Slot (external IRQ source) / DSi: None such
  14    Not used                       / DSi9: NDS-Slot Card change?
  15    Not used                       / DSi: dito for 2nd NDS-Slot?
  16    IPC Sync
  17    IPC Send FIFO Empty
  18    IPC Recv FIFO Not Empty
  19    NDS-Slot Game Card Data Transfer Completion
  20    NDS-Slot Game Card IREQ_MC
  21    NDS9 only: Geometry Command FIFO
  22    NDS7 only: Screens unfolding
  23    NDS7 only: SPI bus
  24    NDS7 only: Wifi    / DSi9: XpertTeak DSP
  25    Not used           / DSi9: Camera
  26    Not used           / DSi9: Undoc, IF.26 set on FFh-filling 40021Axh
  27    Not used           / DSi:  Maybe IREQ_MC for 2nd gamecard?
  28    Not used           / DSi: NewDMA0
  29    Not used           / DSi: NewDMA1
  30    Not used           / DSi: NewDMA2
  31    Not used           / DSi: NewDMA3
  ?     DSi7: any further new IRQs on ARM7 side... in bit13-15,21,25-26?

*/

#define IRQ_MASK_LCD_VBLANK (1 << 0)
#define IRQ_MASK_LCD_HBLANK (1 << 1)
#define IRQ_MASK_LCD_VCOUNTER_MATCH (1 << 2)
#define IRQ_MASK_TIMER_0_OVERFLOW (1 << 3)
#define IRQ_MASK_TIMER_1_OVERFLOW (1 << 4)
#define IRQ_MASK_TIMER_2_OVERFLOW (1 << 5)
#define IRQ_MASK_TIMER_3_OVERFLOW (1 << 6)
#define IRQ_MASK_TIMER_N_OVERFLOW(n) (1 << (3 + (n)))
#define IRQ_MASK_ARM7_SERIAL_IO (1 << 7)
#define IRQ_MASK_DMA_0 (1 << 8)
#define IRQ_MASK_DMA_1 (1 << 9)
#define IRQ_MASK_DMA_2 (1 << 10)
#define IRQ_MASK_DMA_3 (1 << 11)
#define IRQ_MASK_DMA_N(n) (1 << (8 + (n)))
#define IRQ_MASK_KEYPAD (1 << 12)
#define IRQ_MASK_GBA_SLOT (1 << 13)
// 1 << 14 unused
// 1 << 15 unused
#define IRQ_MASK_IPC_SYNC (1 << 16)
#define IRQ_MASK_FIFO_SEND_EMPTY (1 << 17)
#define IRQ_MASK_FIFO_RECEIVE_NOT_EMPTY (1 << 18)
#define IRQ_MASK_GAMECARD_DATA_TRANSFER_DONE (1 << 19)
#define IRQ_MASK_GAMECARD_IREQ_MC (1 << 20) // what is this?
#define IRQ_MASK_ARM9_GEOMETRY_COMMAND_FIFO (1 << 21)
#define IRQ_MASK_ARM7_SCREENS_UNFOLDING (1 << 22)
#define IRQ_MASK_ARM7_SPI_BUS (1 << 23)
#define IRQ_MASK_ARM7_WIFI (1 << 24)

unsigned int SetSpecificInterruptsEnabled(unsigned int which);
// Returns the previous state of interrupts enabled
unsigned int EnableSpecificInterrupts(unsigned int flagMask);
// Returns the previous state of interrupts enabled
unsigned int DisableSpecificInterrupts(unsigned int flagMask);
// Returns the previous state of pending (unacknowledged) interrupts
unsigned int AcknowledgeSpecificInterrupts(unsigned int flagMask);

// All functions return the prior state encoded as an int
// (the old value of cpsr with the relevant bitmask applied).
// You'll often see patterns like
// int oldState = DisableIRQInterrupts();
// ...
// SetIRQInterruptState(oldState);
// to restore a prior state.
int EnableIRQInterrupts();
int DisableIRQInterrupts();
int SetIRQInterruptState(int);
int DisableIRQAndFIQInterrupts();
int SetIRQAndFIQInterruptState(int);
int GetIRQInterruptState();

int GetProcessorMode();