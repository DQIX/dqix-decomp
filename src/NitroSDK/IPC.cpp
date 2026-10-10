#include "System/IPC.h"
#include "System/Interrupts.h"
#include <globaldefs.h>
#include <asmhacks.h>

#pragma optimize_for_size off

#define IPC_FIFO_SYNC (*(volatile unsigned short*)0x04000180)
#define IPC_FIFO_CONTROL (*(volatile unsigned short*)0x04000184)
#define IPC_FIFO_SEND (*(volatile unsigned int*)0x04000188)
#define IPC_FIFO_RECEIVE (*(volatile unsigned int*)0x04100000)

static unsigned short isIPCCommunicationInitialized = false;
static IPCCommandHandler ipcArm9HandlerTable[32];

/*
Fifo control bits:
  Bit   Dir  Expl.
  0     R    Send Fifo Empty Status      (0=Not Empty, 1=Empty)
  1     R    Send Fifo Full Status       (0=Not Full, 1=Full)
  2     R/W  Send Fifo Empty IRQ         (0=Disable, 1=Enable)
  3     W    Send Fifo Clear             (0=Nothing, 1=Flush Send Fifo)
  4-7   -    Not used
  8     R    Receive Fifo Empty          (0=Not Empty, 1=Empty)
  9     R    Receive Fifo Full           (0=Not Full, 1=Full)
  10    R/W  Receive Fifo Not Empty IRQ  (0=Disable, 1=Enable)
  11-13 -    Not used
  14    R/W  Error, Read Empty/Send Full (0=No Error, 1=Error/Acknowledge)
  15    R/W  Enable Send/Receive Fifo    (0=Disable, 1=Enable)
  16-31 -    Not used
  */

union IPCCommand
{
    unsigned int word;
    struct {
        unsigned int channel : 5;
        unsigned int flag : 1;
        unsigned int argument : 26;
    } parts;
};

#define FIFO_CONTROL_MASK_SEND_EMPTY (1 << 0)
#define FIFO_CONTROL_MASK_SEND_FULL (1 << 1)
#define FIFO_CONTROL_MASK_FLUSH_SEND_FIFO (1 << 3)
#define FIFO_CONTROL_MASK_RECEIVE_EMPTY (1 << 8)
#define FIFO_CONTROL_MASK_ENABLE_RECEIVE_FIFO_IRQ (1 << 10)
#define FIFO_CONTROL_MASK_ERROR (1 << 14)
#define FIFO_CONTROL_MASK_ENABLE (1 << 15)

struct Unknown_027ffc00
{
    char unknown[0x388];
    unsigned int ipcHandlersRegistered[2];
};

void InitializeInterProcessorCommunication()
{
    ZeroInitializeIPCCommandHandling();
}

void ZeroInitializeIPCCommandHandling()
{
    int priorState = DisableIRQInterrupts();

    if (!isIPCCommunicationInitialized)
    {
        isIPCCommunicationInitialized = true;
        Unknown_027ffc00* data = (Unknown_027ffc00*)0x027ffc00;
        data->ipcHandlersRegistered[IPCSide_Arm9] = 0;

        int channel = 0;
        do
        {
            ipcArm9HandlerTable[channel] = NULL;
            channel++;
        } while (channel < 32);

        IPC_FIFO_CONTROL = FIFO_CONTROL_MASK_ENABLE | FIFO_CONTROL_MASK_ERROR 
            | FIFO_CONTROL_MASK_ENABLE_RECEIVE_FIFO_IRQ | FIFO_CONTROL_MASK_FLUSH_SEND_FIFO;
        AcknowledgeSpecificInterrupts(IRQ_MASK_FIFO_RECEIVE_NOT_EMPTY);
        SetInterruptHandler(IRQ_MASK_FIFO_RECEIVE_NOT_EMPTY, &HandleCommandReceivedFromArm7);
        EnableSpecificInterrupts(IRQ_MASK_FIFO_RECEIVE_NOT_EMPTY);

        int timeoutCounter;
        unsigned int syncNibble;
        int successfulHandshakeCount = 0;
        while (true)
        {
            syncNibble = IPC_FIFO_SYNC & 0xf;
            IPC_FIFO_SYNC = (syncNibble << 8);
            if (syncNibble == 0 && successfulHandshakeCount > 4)
                break;
            timeoutCounter = 1000;
            if ((IPC_FIFO_SYNC & 0xf) == syncNibble)
            {
                do
                {
                    if (timeoutCounter <= 0)
                    {
                        successfulHandshakeCount = 0;
                        break;
                    }
                    else
                        timeoutCounter--;
                } while ((IPC_FIFO_SYNC & 0xf) == syncNibble);
            }
            successfulHandshakeCount++;
        }
    }

    SetIRQInterruptState(priorState);
}

void SetArm9IPCCommandHandler(int command, IPCCommandHandler handler)
{
    int priorState = DisableIRQInterrupts();
    
    Unknown_027ffc00* data = (Unknown_027ffc00*)0x027ffc00;
    ipcArm9HandlerTable[command] = handler;
    if (handler != NULL)
        data->ipcHandlersRegistered[IPCSide_Arm9] |= (1 << command);
    else
        data->ipcHandlersRegistered[IPCSide_Arm9] &= ~(1 << command);
    
    SetIRQInterruptState(priorState);
}

bool IsIPCCommandHandlerRegistered(int command, IPCSide side)
{
    Unknown_027ffc00* data = (Unknown_027ffc00*)0x027ffc00;
    return data->ipcHandlersRegistered[side] & (1 << command);
}

IPCResult SendCommandToArm7(int channel, int argument, bool flag)
{
    IPCCommand command;
    command.parts.channel = channel;
    command.parts.flag = flag;
    command.parts.argument = argument;

    if (IPC_FIFO_CONTROL & FIFO_CONTROL_MASK_ERROR)
    {
        DECLARE_ASM_NOP();
        IPC_FIFO_CONTROL |=
            FIFO_CONTROL_MASK_ERROR // acknowledge the error
            | FIFO_CONTROL_MASK_ENABLE;
        return IPC_Result_SendError;
    }

    int priorState = DisableIRQInterrupts();

    if (IPC_FIFO_CONTROL & FIFO_CONTROL_MASK_SEND_FULL)
    {
        SetIRQInterruptState(priorState);
        return IPCResult_SendQueueFull;
    }

    IPC_FIFO_SEND = command.word;

    SetIRQInterruptState(priorState);
    return IPCResult_Success;
}

void HandleCommandReceivedFromArm7()
{
    while (true)
    {
        IPCCommand command;
        IPCResult result;
        if (IPC_FIFO_CONTROL & FIFO_CONTROL_MASK_ERROR)
        {
            result = IPCResult_ReceiveError;
            IPC_FIFO_CONTROL |=
            FIFO_CONTROL_MASK_ERROR // acknowledge the error
            | FIFO_CONTROL_MASK_ENABLE;
        }
        else
        {
            int priorState = DisableIRQInterrupts();
            if (IPC_FIFO_CONTROL & FIFO_CONTROL_MASK_RECEIVE_EMPTY)
            {
                SetIRQInterruptState(priorState);
                result = IPCResult_ReceiveQueueEmpty;
            }
            else
            {
                command.word = IPC_FIFO_RECEIVE;
                SetIRQInterruptState(priorState);
                result = IPCResult_Success;
            }
        }

        if (result == IPCResult_ReceiveQueueEmpty)
            break;

        if (result == IPCResult_ReceiveError || command.parts.channel == 0)
            continue;

        if (ipcArm9HandlerTable[command.parts.channel] != 0)
        {
            ipcArm9HandlerTable[command.parts.channel](command.parts.channel, command.parts.argument, command.parts.flag);
        }
        else if (!command.parts.flag)
        {
            command.parts.flag = true;
            if (IPC_FIFO_CONTROL & FIFO_CONTROL_MASK_ERROR)
            {   
                IPC_FIFO_CONTROL |=
                    FIFO_CONTROL_MASK_ERROR // acknowledge the error
                    | FIFO_CONTROL_MASK_ENABLE;
            }
            else
            {
                int priorState = DisableIRQInterrupts();
                if (IPC_FIFO_CONTROL & FIFO_CONTROL_MASK_SEND_FULL)
                {
                    SetIRQInterruptState(priorState);
                }
                else
                {
                    IPC_FIFO_SEND = command.word;
                    SetIRQInterruptState(priorState);
                }
            }
        }
    }
}