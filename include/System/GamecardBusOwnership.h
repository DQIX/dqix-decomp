#pragma once

struct GamecardBusLock
{
    volatile unsigned int atomic;
    unsigned short ownerID;
    unsigned short unknown_6;
};

//void InitializeGamecardBusOwnership();

int ReleaseGBABus(unsigned short owner);
int TryAcquireGBABus(unsigned short owner);

int AcquireNDSBus(unsigned short owner);
int ReleaseNDSBus(unsigned short owner);

unsigned int GenerateLockOwnerID();
// parameter might be unsigned int
void ReleaseLockOwnerID(unsigned short id);