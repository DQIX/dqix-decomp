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
#define NITROVM_OPCODE_SET_FILE_POINTERS 7
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

struct NitroFileAccessor
{
    NitroHandle* handle;
    unsigned int fileID;
};

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

struct FileDataStore
{
    union {
        NitroFileAccessor file;
        NitroDirectoryAccessor dir;
    };
    unsigned int isDirectory; // 1 = directory, 0 = file
    unsigned int stringLength;
    unsigned char name[128];
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
    union
    {
        struct
        {
            void* destination;
            int unknown;
            int length;
        } args_Read;
        struct
        {
            void* source;
            int unknown;
            int length;
        } args_Write;
        struct
        {
            NitroDirectoryAccessor accessor;
        } args_GetDirectoryData;
        struct
        {
            FileDataStore* output;
            CBool skipStoreString;
        } args_GetFileOrDirectoryNameData;
        struct
        {
            // the directory to search in
            NitroDirectoryAccessor searchDirectory;
            const unsigned char* path;
            CBool searchForDirectory;
            void* volatile output;
        } args_GetFileOrDirectoryByName;
        struct
        {
            void* pathOutput;
            int outputCapacity;
            // when calling this manually, you should set this to zero 
            unsigned short numBytesWritten;
            // when calling this manually, you should set this to zero.
            // if you specified a directory, this will be populated with that ID,
            // if you specified a file this will be populated by the ID of the
            // directory containing it after this command runs
            unsigned short directoryID;
        } args_GetPath;
        struct
        {
            NitroFileAccessor accessor;
        } args_GetFATEntry;
        struct
        {
            int startOffset;
            int endOffset;
            int fileID;
        } args_SetFilePointers;
    };
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

void NitroVM_UnlinkAndStoreResult(NitroVM* fs, int result);
int NitroVM_ExecuteCommand(NitroVM* fs, int opcode);
int CaseInsensitiveStrncmp(const unsigned char* first, const unsigned char* second, unsigned int len);
int Nitro_ReadMetadataBytes(FSReadDescription* handle, void* dst, unsigned int len);

// Pass the index of the directory relative to the beginning of directory entries.
// That is, for the directory F000, pass 0, for FFFF, pass FFF.
int NitroVM_LoadDirectoryDataByIndex(NitroVM* fs, unsigned int dirIndex);

// Default implementation for NitroVM opcode 0.
int NitroVM_DefaultCommand_Read(NitroVM* fs);

// Default implementation for NitroVM opcode 1.
// Very likely completely unused since nitro files are read-only
int NitroVM_DefaultCommand_Write(NitroVM* fs);

// Default implementation for NitroVM opcode 2.
int NitroVM_DefaultCommand_GetDirectoryData(NitroVM* fs);

// Default implementation for NitroVM opcode 3.
// Loads data from a FNT subtable within the nitro filesystem.
int NitroVM_DefaultCommand_GetFileOrDirectoryNameData(NitroVM* fs);

// Default implementation for NitroVM opcode 4.
// Gets the ID and related data of a file / directory based on the name.
int NitroVM_DefaultCommand_GetFileOrDirectoryByName(NitroVM* fs);

// Default implementation for NitroVM opcode 5.
// Computes the full path of a file or directory given its ID.
int NitroVM_DefaultCommand_GetPath(NitroVM* fs);

// Default implementation for NitroVM opcode 6.
// Loads data from the file allocation table.
int NitroVM_DefaultCommand_GetFATEntry(NitroVM* fs);

// Default implementation for NitroVM opcode 7.
// Loads the file info to set it up for reading from the start.
int NitroVM_DefaultCommand_SetFilePointers(NitroVM* fs);

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