#pragma once

#include "System/ProcessorContext.h"

template<class T>
struct FSListHeader
{
    T* pPrev;
    T* pNext;
};

struct NitroHandle;
struct NitroVM;

typedef int CBool;

// The FS72 struct seems to hold a bunch of values that are used
// for various different purposes by different commands. It seems they're
// essentially treated as registers for some kind of virtual machine
union FSRegister
{
    void* ptr;
    int s32;
    unsigned int u32;
    struct {
        unsigned short low;
        unsigned short high;
    } u16;
    struct {
        short low;
        short high;
    } s16;
};

// There are a few operations that act on multiple registers at once
// via ldm / stm commands, which is consistent with copy-assigning an array
union FSRegisterTriple
{
    struct {
        FSRegister a;
        FSRegister b;
        FSRegister c;
    };
};

#define NITROVM_FLAG_IN_HANDLE_QUEUE 0
#define NITROVM_FLAG_MARKED_FOR_UNLINK_FROM_HANDLE 1
#define NITROVM_FLAG_SYNCHRONOUS 2
#define NITROVM_FLAG_EXECUTING_FROM_QUEUE 3
#define NITROVM_FLAG_READ_POSITIONS_CONFIGURED 4
// might have other uses but this is the only decomped one so far
#define NITROVM_FLAG_SEARCH_TARGET_IS_DIRECTORY 5
#define NITROVM_FLAG_READY_TO_EXECUTE 6

#define NITROVM_OPCODE_LOAD 0
#define NITROVM_OPCODE_SAVE 1
#define NITROVM_OPCODE_GET_DIRECTORY_DATA 2
#define NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA 3
#define NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_BY_NAME 4
#define NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_PATH 5
#define NITROVM_OPCODE_GET_FAT_ENTRY 6
#define NITROVM_OPCODE_COPY_REGISTERS 7
#define NITROVM_OPCODE_NOP 8
#define NITROVM_OPCODE_ACQUIRE_NDS_BUS 9
#define NITROVM_OPCODE_RELEASE_NDS_BUS 10
#define NITROVM_OPCODE_MAYBE_INVALID 14

#define NITRO_RESULT_SUCCESS 0
#define NITRO_RESULT_FAILURE 1
#define NITRO_RESULT_UNDEFINED 2
#define NITRO_RESULT_INVALID_HANDLE 3
#define NITRO_RESULT_COMMAND_UNSUPPORTED 4
#define NITRO_RESULT_5 5
// shows up e.g. when reading bytes for the ROM filesystem, as that requires
// commands to be sent to the gamecard and this takes place on a different thread.
#define NITRO_RESULT_TASK_STILL_RUNNING 6
#define NITRO_RESULT_FALLBACK_TO_DEFAULT 7
#define NITRO_RESULT_OPCODE_NOT_IMPLEMENTED 8

struct NitroDirectoryAccessor
{
    // unnamed struct to make this trivially copyable (using ldm, stm commands)
    struct {
        NitroHandle* handle;
        unsigned short dirID;
        unsigned short firstFileID;
        unsigned int handleSubtableOffset;
    };
};

// sizeof(NitroVM) == 72 == 0x48.
// Used to execute 'commands' on a handle in order to load files. e.g. There
// are commands to read bytes based on offsets within the struct, and commands
// to set those in preparation for reading a specific file.
struct NitroVM
{
    FSListHeader<NitroVM> links;
    NitroHandle* linkedHandle;
    volatile unsigned int flags;
    int pendingCommand;
    int storedResult;
    BlockedContextList blockedContexts;
    union
    {
        struct
        {
            NitroDirectoryAccessor accessor;
            unsigned int parentID;
        } dirInfo;
        struct
        {
            unsigned int fileID;
            int startOffset; // offsets relative to the start of the
            int endOffset;   // archive the file is in. Notably, 
            int cursorPos;   // startOffset <= cursorPos <= endOffset.
        } fileInfo;
    };
    FSRegisterTriple regext_abc;
    FSRegister regext_d;
    FSRegister reg8;
    volatile FSRegister reg9;
};

struct FSReadDescription
{
    NitroHandle* nitroHandle;
    unsigned int offset;
};

#define NITROHANDLE_FLAG_IS_IN_MAIN_LIST 0
#define NITROHANDLE_FLAG_IS_POPULATED 1
#define NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY 2
#define NITROHANDLE_FLAG_QUEUE_PAUSED 3
#define NITROHANDLE_FLAG_NDS_BUS_HELD 4
#define NITROHANDLE_FLAG_VM_LIST_DIRTY 5
#define NITROHANDLE_FLAG_AWAITING_BUS_RELEASE 6
#define NITROHANDLE_FLAG_DESTRUCTION_UNDERWAY 7
#define NITROHANDLE_FLAG_ASYNC_COMMAND_IN_PROGRESS 8
#define NITROHANDLE_FLAG_SYNC_COMMAND_IN_PROGRESS 9

// sizeof(NitroHandle) == 92 == 0x5C.
// Represents a handle to a filesystem (e.g. the one on the ROM, or the 
// files within a .narc file that's been loaded into memory already).
struct NitroHandle
{
    typedef int(*ReadProc)(NitroHandle*, void*, unsigned int, unsigned int);
    typedef int(*WriteProc)(NitroHandle*, const void*, unsigned int, unsigned int);
    typedef int(*CommandOverride)(NitroVM*, int);

    unsigned int signature; // 'rom' or 'arc'
    // All global handles are held in a linked list so you can search for a file
    // by e.g. "rom:/path/to/file" or "arc:/path/to/file" and get the right
    // handle based on the signature
    NitroHandle* pNextHandle;
    NitroHandle* pPrevHandle;
    BlockedContextList taskWaitBlock;
    BlockedContextList busReleaseBlock;
    volatile unsigned int flags;
    // Creates a fake first entry in a list of VMs. The previous pointer
    // in here is unused (probably null). Use linkToFirstVM.pNext for the
    // first legitimate entry
    FSListHeader<NitroVM> linkToFirstVM;
    void* pFileImage;
    int fatOffsetFast;
    unsigned int fatSize;
    int nameTableOffsetFast;
    unsigned int nameTableSize;
    int fatOffset;
    int nameTableOffset;
    // When the tables are loaded into memory, this is the pointer that was
    // passed (though the start of the tables might be a bit after this)
    void* tableRawPointer;
    ReadProc readProc;
    WriteProc writeProc;
    // You can load the metadata (file allocation table + name tables) into
    // memory, then this proc allows for reading it directly from that memory
    // block. When not loaded into memory, fastReadProc == readProc
    ReadProc fastReadProc;
    // Some types of handle need to behave differently for certain instructions,
    // for example the handle for the ROM filesystem needs to acquire the 
    // gamecard bus from the ARM7 before it can read
    CommandOverride instructionOverride;
    // if (1 << n) bit is set, use the above override for command n
    unsigned int overrideOpcodeFlags;
};

struct NitroFileAccessor
{
    NitroHandle* handle;
    unsigned int fileID;
};


void NitroVM_UnlinkAndStoreResult(NitroVM* fs, int result);
int NitroVM_ExecuteCommand(NitroVM* fs, int opcode);
int CaseInsensitiveStrncmp(const unsigned char* first, const unsigned char* second, unsigned int len);
int Nitro_ReadMetadataBytes(FSReadDescription* handle, void* dst, unsigned int len);

// Pass the index of the directory relative to the beginning of directory entries.
// That is, for the directory F000, pass 0. For FFFF, pass FFF.
// The FS struct will end up containing the following values.
// base_A: NitroHandle* (fs->unknown_8)
// base_B.low = input dirIndex
// base_B.high = id of first file in this directory
// base_C = offset of subtable relative to start of data section for NarcHandleInitial
// base_D = (int)id of parent directory
int NitroVM_LoadDirectoryDataByIndex(NitroVM* fs, unsigned int dirIndex);

// Default implementation for NitroVM opcode 0.
// Expects as inputs:
// base_D = offset from beginning of the nitro data to start copying from
// ext_A = destination to load data to
// ext_C = number of bytes to load.
// Outputs:
// base_D is adjusted by the number of bytes loaded.
int NitroVM_DefaultCommand_Read(NitroVM* fs);

// Default implementation for NitroVM opcode 1.
// Very likely completely unused since nitro files are read-only.
// Expects as inputs:
// base_D = offset from beginning of the nitro data to start copying to
// ext_A = source data to copy
// ext_C = number of bytes to copy
// Outputs:
// value_2C is adjusted by the number of bytes saved.
int NitroVM_DefaultCommand_Write(NitroVM* fs);

// Default implementation for NitroVM opcode 2.
// Expects as inputs:
// ext_B_low = directory index (omit the 0xF000)
// Outputs:
// base_A = input ext_A
// base_B_low = input directory index
// base_D = parent directory index, or num directories if this is the root
// Provided that ext_C = 0 and ext_B_high = 0:
// base_B_high is overwritten by the first file in the referenced directory
// base_C is the offset to the subtable for files/subdirs in said directory
// (relative to the nitro handle)
// If this precondition is not met, the values from ext_b_high and ext_c
// respectively will just be copied in
int NitroVM_DefaultCommand_GetDirectoryData(NitroVM* fs);

// Default implementation for NitroVM opcode 3.
// Loads data from a FNT subtable within the nitro filesystem.
// Inputs:
// base_C = offset to start reading from (relative to the nitro handle)
// ext_b = (int as bool) 1 if string should *NOT* be saved, 0 otherwise
// ext_a = pointer to destination for extra data here
// base_b_high = probably: index of current file to process (if this entry is a file)
// Outputs:
// The address in ext_a is filled as a FileDataStore (see the cpp file)
// base_C is updated to the new offset after this entry
// base_b_high is incremented if we found a file (not a directory)
// Returns 0 on success, 1 if at end of table and other values if inner
// processes go wrong for whatever reason
int NitroVM_DefaultCommand_GetFileOrDirectoryNameData(NitroVM* fs);

// Default implementation for NitroVM opcode 4.
// Gets the ID and related data of a file / directory based on the name.
// Inputs:
// [technicality] ext_a = should equal nitro handle if passing empty file string, unused otherwise
// ext_b_low = id of root directory to start the search, minus 0xF000. Zero means search everywhere
// ext_b_high = should be zero to avoid weird behaviour
// ext_c = should be zero to avoid weird behaviour
// ext_d = pointer to buffer holding file path
// reg8 = 1 if searching for a directory, 0 if searching for a file
// reg9 = storage pointer
// Outputs:
// At the address given by the reg9 input, we populate either a
// NitroFileMetadata or a NitroDirectoryMetadata depending on what we
// found.
// The other registers are probably unused, but their contents will be:
// base_a = nitro handle
// base_b_low = id either of the directory we found, or of the directory
//              containing the file we found
// base_b_high = first file of directory, or 1+(id of found file)
// base_c = offset to FNT subtable for found directory, or pointer past
//          entry in the relevant FNT subtable for the file we found
// base_d = parent directory id of either the found directory or the directory
//          containing the file we found (i.e. 2 levels up)
// ext_a = nitro handle if got a directory, garbage stack pointer otherwise
// ext_b = directory id if got a directory, 0 otherwise
// ext_c = 0
// ext_d, reg8, reg9 unchanged
int NitroVM_DefaultCommand_GetFileOrDirectoryByName(NitroVM* fs);

// Default implementation for NitroVM opcode 5.
// Computes the full path of a file or directory given its ID.
// Inputs:
// flag bit 5 (0x20): set if specifying a directory, clear if specifying a file
// base_a: id of file to get data of (unused if handling a directory)
// base_b_low: id of dir to get data of (unused for files)
// ext_a: pointer to output the filename. If null, nothing will be written
//        and routine will succeed, providing only the other outputs
// ext_b: output buffer capacity
// ext_c_low: should be 0. If nonzero, this and ext_c_high will be treated
//            as outputs from a previous run of this function (with only
//            perhaps a different value of ext_a)
// Outputs:
// the full path, as a null-terminated string in the format
// "sig:/path/to/file.ext" (in practice, sig is either 'arc' or 'rom')
// is written to the address in ext_a
// ext_c_low: number of bytes written, including the null-terminator
// ext_c_high: directory id (either input dir id if you specified a dir, or
// the id of the container directory if you specified a file)
int NitroVM_DefaultCommand_GetPath(NitroVM* fs);

// Default implementation for NitroVM opcode 6.
// Loads data from the file allocation table.
// Inputs:
// ext_b: file id / index in the table
// Outputs:
// base_a and ext_c: file id
// base_b, base_d and ext_a: beginning of allocation
// base_c and ext_b: end of allocation
int NitroVM_DefaultCommand_GetFATEntry(NitroVM* fs);

// Default implementation for NitroVM opcode 7.
// Copies data from the extended registers into the base registers.
// Specifically:
// ext_a is copied into base_b and base_d
// ext_b is copied into base_c
// ext_c is copied into base_a
int NitroVM_DefaultCommand_CopyRegisters(NitroVM* fs);

// Default implementation for NitroVM opcode 8.
// Does nothing, but might be intended as a sort of shutdown/destructor.
// (See e.g. func_020cca80 in USA, which is used at the end of functions
// that create temporary VMs and invokes command 8).
int NitroVM_DefaultCommand_Nop(NitroVM* fs);

unsigned int Nitro_CalculateSignature(const char* str, int len);

NitroVM* NitroHandle_AdvanceCommandQueue(NitroHandle* handle);
void NitroVM_ProcessReadyCommandQueueEntries(NitroVM* vm);
CBool NitroVM_ExecuteAndUnlink(NitroVM* vm);
// Queues the command, and if the VM is synchronous, waits for it to complete.
// Note that operands 2 through 8 (i.e. everything except reading) is always
// treated as being synchronous.
CBool NitroVM_QueueCommand(NitroVM* vm, int opcode);