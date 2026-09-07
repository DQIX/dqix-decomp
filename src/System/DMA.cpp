#include "System/DMA.h"
#include "System/Interrupts.h"
#include <globaldefs.h>

#pragma optimize_for_size off

#if defined(jpn)
#define func_020c9be0 func_020cb6ac
#endif

extern "C"
{
    // abort() or similar
    void func_020c9be0();
}

void VerifyDMASource(int channel, unsigned int source, unsigned int length, unsigned int sourceCtrlFlags);

#define DMA_REGISTER_ADDR_BASE 0x040000b0
#define DMA_REGISTER_FILL_BASE 0x040000e0

#define DMA_CONTROL_DST_ADDR_INCREMENT (0u << 21)
#define DMA_CONTROL_DST_ADDR_DECREMENT (1u << 21)
#define DMA_CONTROL_DST_ADDR_FIXED (2u << 21)
#define DMA_CONTROL_DST_ADDR_RELOAD (3u << 21)
#define DMA_CONTROL_SRC_ADDR_INCREMENT (0u << 23)
#define DMA_CONTROL_SRC_ADDR_DECREMENT (1u << 23)
#define DMA_CONTROL_SRC_ADDR_FIXED (2u << 23)
#define DMA_CONTROL_SRC_ADDR_PROHIBITED (3u << 23)
#define DMA_CONTROL_REPEAT (1u << 25)
#define DMA_CONTROL_TRANSFER_32_BIT (1u << 26)
#define DMA_CONTROL_START_TIMING_IMMEDIATE (0u << 27)
#define DMA_CONTROL_START_TIMING_VBLANK (1u << 27)
#define DMA_CONTROL_START_TIMING_HBLANK (2u << 27)
#define DMA_CONTROL_START_TIMING_SYNC_DISPLAY_START (3u << 27)
#define DMA_CONTROL_START_TIMING_MAIN_MEMORY_DISPLAY (4u << 27)
#define DMA_CONTROL_START_TIMING_DS_CARTRIDGE_SLOT (5u << 27)
#define DMA_CONTROL_START_TIMING_GBA_CARTRIDGE_SLOT (6u << 27)
#define DMA_CONTROL_START_TIMING_GEOMETRY_FIFO (7u << 27)
#define DMA_CONTROL_IRQ_AT_END (1u << 30)
#define DMA_CONTROL_ENABLE (1u << 31)

#define DMA_CONTROL_MASK_START_TIMING (7u << 27)

inline volatile DMARegisterSet& DMARegisters(int n)
{
    return *(volatile DMARegisterSet*)(n * 12 + DMA_REGISTER_ADDR_BASE);
}

inline volatile unsigned int& DMARegisterSourceAddr(int n)
{
    return *(volatile unsigned int*)(n * 12 + DMA_REGISTER_ADDR_BASE);
}

inline volatile unsigned int& DMARegisterDestAddr(int n)
{
    return *(&DMARegisterSourceAddr(n) + 1);
}

inline volatile unsigned int& DMARegisterControl(int n)
{
    return *(volatile unsigned int*)((n * 3 + 2) * 4 + DMA_REGISTER_ADDR_BASE);
}

inline volatile unsigned short& DMARegisterControlLow(int n)
{
    return *(volatile unsigned short*)((n * 6 + 4) * 2 + DMA_REGISTER_ADDR_BASE);
}

inline volatile unsigned short& DMARegisterControlHigh(int n)
{
    return *(volatile unsigned short*)((n * 6 + 5) * 2 + DMA_REGISTER_ADDR_BASE);
}

inline volatile unsigned int& DMARegisterFill(int n)
{
    return *(volatile unsigned int*)(n * 4 + DMA_REGISTER_FILL_BASE);
}

void DMAMemsetSynchronous(int channel, unsigned int dst, unsigned int value, unsigned int len)
{
    if (len == 0)
        return;
    while (DMARegisterControl(channel) & DMA_CONTROL_ENABLE) {}

    int priorState = DisableIRQInterrupts();
    DMARegisterFill(channel) = value;
    ConfigureDMATransferSafe(channel, (unsigned int)&DMARegisterFill(channel), dst, 
        (len >> 2) | DMA_CONTROL_ENABLE | DMA_CONTROL_TRANSFER_32_BIT |
        DMA_CONTROL_SRC_ADDR_FIXED | DMA_CONTROL_DST_ADDR_INCREMENT);
    SetIRQInterruptState(priorState);

    while (DMARegisterControl(channel) & DMA_CONTROL_ENABLE) {}
}

void DMAMemcpySynchronous(int channel, unsigned int src, unsigned int dst, unsigned int len)
{
    VerifyDMASource(channel, src, len, DMA_CONTROL_SRC_ADDR_INCREMENT);
    if (len == 0)
        return;

    // Wait for previous operation to finish
    while (DMARegisterControl(channel) & DMA_CONTROL_ENABLE) {}

    ConfigureDMATransferSafeAtomic(channel, src, dst, (len >> 2) | 
        DMA_CONTROL_ENABLE | DMA_CONTROL_TRANSFER_32_BIT |
         DMA_CONTROL_SRC_ADDR_INCREMENT | DMA_CONTROL_DST_ADDR_INCREMENT);

    // Wait for this operation to finish
    while (DMARegisterControl(channel) & DMA_CONTROL_ENABLE) {}
}

void DMAMemcpySynchronous16Bit(int channel, unsigned int src, unsigned int dst, unsigned int len)
{
    if (len == 0)
        return;
    VerifyDMASource(channel, src, len, DMA_CONTROL_SRC_ADDR_INCREMENT);

    // Wait for previous operation to finish
    while (DMARegisterControl(channel) & DMA_CONTROL_ENABLE) {}

    ConfigureDMATransferSafeAtomic(channel, src, dst, (len >> 1) | DMA_CONTROL_ENABLE |
        DMA_CONTROL_SRC_ADDR_INCREMENT | DMA_CONTROL_DST_ADDR_INCREMENT);

    // Wait for this operation to finish
    while (DMARegisterControl(channel) & DMA_CONTROL_ENABLE) {}
}

void DMAMemsetAsync(int channel, unsigned int dst, unsigned int value, unsigned int len,
    DMACompletionCallback onCompletion, int callbackUserdata)
{
    if (len == 0)
    {
        if (onCompletion != NULL)
            onCompletion(callbackUserdata);
    }
    else
    {
        AwaitDMACompletion(channel);
        if (onCompletion != NULL)
        {
            SetDMACompletionCallback(channel, onCompletion, callbackUserdata);
            int priorState = DisableIRQInterrupts();
            DMARegisterFill(channel) = value;
            ConfigureDMATransfer(channel, (unsigned int)&DMARegisterFill(channel), dst,
                (len >> 2) | DMA_CONTROL_ENABLE |
                DMA_CONTROL_IRQ_AT_END | DMA_CONTROL_TRANSFER_32_BIT |
                DMA_CONTROL_SRC_ADDR_FIXED | DMA_CONTROL_DST_ADDR_INCREMENT);
            SetIRQInterruptState(priorState);
        }
        else
        {
            int priorState = DisableIRQInterrupts();
            DMARegisterFill(channel) = value;
            ConfigureDMATransfer(channel, (unsigned int)&DMARegisterFill(channel), dst,
                (len >> 2) | DMA_CONTROL_ENABLE | DMA_CONTROL_TRANSFER_32_BIT |
                DMA_CONTROL_SRC_ADDR_FIXED | DMA_CONTROL_DST_ADDR_INCREMENT);
            SetIRQInterruptState(priorState);
        }
    }
}

void DMAMemcpyAsync(int channel, unsigned int src, unsigned int dst, unsigned int len,
    DMACompletionCallback onCompletion, int callbackUserdata)
{
    VerifyDMASource(channel, src, len, DMA_CONTROL_SRC_ADDR_INCREMENT);
    if (len == 0)
    {
        if (onCompletion != NULL)
            onCompletion(callbackUserdata);
    }
    else
    {
        AwaitDMACompletion(channel);
        if (onCompletion != NULL)
        {
            SetDMACompletionCallback(channel, onCompletion, callbackUserdata);
            ConfigureDMATransferAtomic(channel, src, dst,
                (len >> 2) | DMA_CONTROL_ENABLE |
                DMA_CONTROL_IRQ_AT_END | DMA_CONTROL_TRANSFER_32_BIT |
                DMA_CONTROL_SRC_ADDR_INCREMENT | DMA_CONTROL_DST_ADDR_INCREMENT);
        }
        else
        {
            ConfigureDMATransferAtomic(channel, src, dst,
                (len >> 2) | DMA_CONTROL_ENABLE | DMA_CONTROL_TRANSFER_32_BIT |
                DMA_CONTROL_SRC_ADDR_INCREMENT | DMA_CONTROL_DST_ADDR_INCREMENT);
        }
    }
}

void AwaitDMACompletion(int id)
{
    int priorState = DisableIRQInterrupts();
    while (DMARegisterControl(id) & DMA_CONTROL_ENABLE) {}

    if (id == 0)
    {
        volatile DMARegisterSet& regs = DMARegisters(id);
        regs.sourceAddr = 0;
        regs.destAddr = 0;
        regs.controlAndSize.u32 = DMA_CONTROL_ENABLE | DMA_CONTROL_SRC_ADDR_FIXED | DMA_CONTROL_DST_ADDR_FIXED | 1;
    }
    SetIRQInterruptState(priorState);
}

void ResetDMAChannel(int id)
{
    int priorState = DisableIRQInterrupts();

    DMARegisterControlHigh(id) &= ~((DMA_CONTROL_MASK_START_TIMING | DMA_CONTROL_REPEAT) >> 16);
    DMARegisterControlHigh(id) &= ~(DMA_CONTROL_ENABLE >> 16);

    // GBATek says something about waiting 2 cycles, but only for enabling DMA
    (void)DMARegisterControlHigh(id);
    (void)DMARegisterControlHigh(id);

    if (id == 0)
    {
        volatile DMARegisterSet& regs = DMARegisters(id);
        regs.sourceAddr = 0;
        regs.destAddr = 0;
        regs.controlAndSize.u32 = DMA_CONTROL_ENABLE | DMA_CONTROL_SRC_ADDR_FIXED | DMA_CONTROL_DST_ADDR_FIXED | 1;
    }

    SetIRQInterruptState(priorState);
}

void ResetAllDMAChannels()
{
    ResetDMAChannel(0);
    ResetDMAChannel(1);
    ResetDMAChannel(2);
    ResetDMAChannel(3);
}

// Speculative: checks if swapping the specified channel's timing is going to
// work, and aborts if not. The swap is valid iff all other enabled channels either
//     - use immediate timing
//     - use the same timing that we want to switch it
//     - if newTiming = VBlank, then HBlank timing is permitted 
//     - if newTiming = HBlank, then VBlank timing is permitted
// Note that only channels 0, 1, 2 are checked.
void VerifyDMATimingChangePermitted_020c9fd0(int channel, unsigned int newTimingFlags)
{
    int loopChannel = 0;
    volatile unsigned int* pControl = &DMARegisterControl(0);
    
    do {
        if (loopChannel == channel)
            continue;

        unsigned int control = *pControl;


        if (!(control & DMA_CONTROL_ENABLE) ||
            (control & DMA_CONTROL_MASK_START_TIMING) == newTimingFlags)
            continue;

        unsigned int loopChannelTiming = control & DMA_CONTROL_MASK_START_TIMING;
        if (loopChannelTiming == DMA_CONTROL_START_TIMING_VBLANK && newTimingFlags == DMA_CONTROL_START_TIMING_HBLANK)
            continue;

        if (loopChannelTiming == DMA_CONTROL_START_TIMING_HBLANK && newTimingFlags == DMA_CONTROL_START_TIMING_VBLANK)
            continue;

        if (loopChannelTiming != DMA_CONTROL_START_TIMING_SYNC_DISPLAY_START &&
            loopChannelTiming != DMA_CONTROL_START_TIMING_MAIN_MEMORY_DISPLAY &&
            loopChannelTiming != DMA_CONTROL_START_TIMING_DS_CARTRIDGE_SLOT &&
            loopChannelTiming != DMA_CONTROL_START_TIMING_GBA_CARTRIDGE_SLOT &&
            loopChannelTiming != DMA_CONTROL_START_TIMING_GEOMETRY_FIFO &&
            loopChannelTiming != DMA_CONTROL_START_TIMING_VBLANK &&
            loopChannelTiming != DMA_CONTROL_START_TIMING_HBLANK)
            continue;
        
        
        func_020c9be0();
    } while (loopChannel++, pControl += 3, loopChannel < 3);
}

// It seems that channel 0 is not permitted to read from anywhere
// in the range 04xxxxxx (I/O registers) or beyond 08000000 (GBA stuff).
// This function doesn't place restrictions on the other channels, and in fact
// doing DMA from GAMECARD_RECEIVED_DATA (0x04100010) is one way to load
// from the gamecard
void VerifyDMASource(int channel, unsigned int source, unsigned int length, unsigned int sourceCtrlFlags)
{
    if (channel != 0)
        return;

    unsigned int startPtrSection = source & 0xff000000;

    if (sourceCtrlFlags != DMA_CONTROL_SRC_ADDR_INCREMENT)
    {
        if (sourceCtrlFlags == DMA_CONTROL_SRC_ADDR_DECREMENT)
        {
            source -= length;
        }
    }
    else 
        source += length;
    unsigned int endPtrSection = source & 0xff000000;

    if (startPtrSection == 0x04000000 || startPtrSection >= 0x08000000 ||
        endPtrSection == 0x04000000 || endPtrSection >= 0x08000000)
        func_020c9be0();
}