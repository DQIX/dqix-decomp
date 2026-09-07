#pragma once

#include "NitroVM.h"

CBool CreateFileAccessor(NitroFileAccessor* outAccessor, const char* path);

// I don't really know what the 5th parameter is there for, but in practice it's always -1
CBool NitroVM_PrepareRead(NitroVM* vm, NitroHandle* handle, unsigned int start, unsigned int end, unsigned int fileID);

// Sets up the appropriate registers to point to the relevant parts, so that 
// the handle load proc can load the file
bool NitroVM_PrepareReadFileByID(NitroVM* vm, NitroFileAccessor accessor);

bool NitroVM_PrepareReadFileByPath(NitroVM* vm, const char* path);

// I don't know what this function actually does, but I think it has 
// some synchronisation purpose. It is often called at destruction time,
// but it also sometimes gets called in the middle of a function (see e.g.
// func_020ccffc in USA). 
CBool NitroVM_FinishRead(NitroVM* vm);

// Assuming that register base_A holds the file ID, runs command 5 to output
// the filename to the specified buffer
void NitroVM_WriteOutFilePath(NitroVM* vm, char* buffer, unsigned int bufferLength);

CBool NitroVM_AwaitCommandCompletion(NitroVM* vm);
void NitroVM_CancelCommand(NitroVM* vm);

int NitroVM_ReadAsync(NitroVM*, void* dst, int capacity);
int NitroVM_ReadSync(NitroVM*, void* dst, int capacity);

// this is (probably) like libc fseek(). Assumes that base_b
// holds the start address, base_c the end address and base_d the
// current position, and updates base_d appropriately.
// 0 = SEEK_SET (from start), 1 = SEEK_CUR (adjust), 2 = SEEK_END
CBool NitroVM_Seek(NitroVM* vm, int position, int whence);

CBool SetROMFilesystemRoot(const char* path);

void OnCartridgeDataLoadCompletion(NitroHandle*);
int ROMFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned);
int ROMFilesystemSaveProc(NitroHandle*, const void*, unsigned, unsigned); // doesn't do anything!

int ROMFilesystemOpcodeOverride(NitroVM*, int opcode);

int UnsupportedFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned);
int UnsupportedFilesystemOpcodeOverride(NitroVM*, int opcode);

void InitializeROMFilesystem_Internal(unsigned int unknown);

unsigned int ChangeROMLoadingDMAChannel(unsigned int newChannel);

// Load file tables into memory for the primary nitro handle (ROM FS),
// provided the buffer is large enough. Returns the amount of space needed.
unsigned int LoadROMFilesystemFileTables(void* where, unsigned int bufferLen);