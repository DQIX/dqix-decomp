#pragma once

enum IPCResult
{
    IPCResult_Success = 0,
    IPC_Result_SendError = -1, // generic error yet to be acknowledged
    IPCResult_SendQueueFull = -2,
    IPCResult_ReceiveError = -3, // generic error yet to be acknowledged
    IPCResult_ReceiveQueueEmpty = -4
};

enum IPCSide
{
    IPCSide_Arm9 = 0,
    IPCSide_Arm7 = 1
};

typedef void (*IPCCommandHandler)(unsigned int, unsigned int, unsigned int);

void InitializeInterProcessorCommunication();
void ZeroInitializeIPCCommandHandling();

void SetArm9IPCCommandHandler(int command, IPCCommandHandler handler);
bool IsIPCCommandHandlerRegistered(int command, IPCSide side);

IPCResult SendCommandToArm7(int command, int argument, bool flag);
void HandleCommandReceivedFromArm7(); // used as IRQ 18 handler
