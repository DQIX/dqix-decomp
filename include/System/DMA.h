#pragma once

typedef void(*DMACompletionCallback)(int userdata);

struct DMARegisterSet
{
    unsigned int sourceAddr;
    unsigned int destAddr;
    union {
        unsigned int u32;
        struct {
            unsigned short transferSize;
            unsigned short control;
        };
    } controlAndSize;
};

// ITCM functions. The atomic operations are wrapped with DisableIRQInterrupts()
// and SetIRQInterruptState(), 
void ConfigureDMATransferAtomic(int channel, unsigned src, unsigned dst, unsigned ctrl);
void ConfigureDMATransferSafeAtomic(int channel, unsigned src, unsigned dst, unsigned ctrl);
void ConfigureDMATransfer(int channel, unsigned src, unsigned dst, unsigned ctrl);
void ConfigureDMATransferSafe(int channel, unsigned src, unsigned dst, unsigned ctrl);

void DMAMemsetSynchronous(int channel, unsigned int dst, unsigned int value, unsigned int len);
void DMAMemcpySynchronous(int channel, unsigned int src, unsigned int dst, unsigned int len);
void DMAMemcpySynchronous16Bit(int channel, unsigned int src, unsigned int dst, unsigned int len);

// Because immediate mode DMA transfers block the CPU until done, this isn't
// actually async, just set up to look like it
void DMAMemsetAsync(int channel, unsigned int dst, unsigned int value, unsigned int len,
    DMACompletionCallback onCompletion, int callbackUserdata);
void DMAMemcpyAsync(int channel, unsigned int src, unsigned int dst, unsigned int len,
    DMACompletionCallback onCompletion, int callbackUserdata);

void AwaitDMACompletion(int channel);
void ResetDMAChannel(int channel);
void ResetAllDMAChannels();

void VerifyDMATimingChangePermitted_020c9fd0(int channel, unsigned int newTimingFlags);
void VerifyDMASource(int channel, unsigned int source, unsigned int length, unsigned int sourceCtrlFlags);