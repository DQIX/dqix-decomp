#pragma once

#include "NitroVM.h"

void NitroHandle_Initialize(NitroHandle*);
NitroHandle* NitroHandle_FindBySignature(const char* str, int len);
CBool NitroHandle_AddToHandleList(NitroHandle*, const char* sig, int len);
void NitroHandle_RemoveFromHandleList(NitroHandle*);

int NitroHandle_Populate(NitroHandle*, void* image,
    unsigned int fatOffset, unsigned int fatSize,
    unsigned int fntOffset, unsigned int fntSize,
    NitroHandle::ReadProc readProc, NitroHandle::WriteProc writeProc);

CBool NitroHandle_Destroy(NitroHandle*);

// Loads the file tables (FAT and FNT) into memory at the given
// position. The memory should already be allocated in some way.
// Returns the amount of space used or needed. If this exceeds the
// capacity, nothing will be written.
unsigned int NitroHandle_LoadFileTables(NitroHandle*, void* into, unsigned int capacity);

// Removes all references to the file tables, then returns the pointer
// to the allocation so that you can free the memory
void* NitroHandle_ReleaseFileTables(NitroHandle*);

CBool NitroHandle_Pause(NitroHandle*);
CBool NitroHandle_Unpause(NitroHandle*);

void NitroHandle_SetOpcodeOverride(NitroHandle*, NitroHandle::CommandOverride, unsigned int mask);

void NitroHandle_OnTaskCompletion(NitroHandle*, int result);

void InitializeCartridgeFilesystem(int unknown);

void NitroVM_Initialize(NitroVM*);

// Can be used with a temporary VM, and doesn't need a nitro handle
// attached - it will be assigned by the function itself
int NitroVM_SearchFileOrDirectory(NitroVM*, const char* path,
    NitroFileAccessor* outFileData, NitroDirectoryAccessor* outDirData);

// Returns number of bytes loaded
int NitroVM_Read(NitroVM*, void* dst, int capacity, CBool async);