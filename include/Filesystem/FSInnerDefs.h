#pragma once

#include "NitroVM.h"
#include "LowNitroHandle.h"
#include "FileAccessor.h"
#include "CardReadManager.h"
#include "System/ProcessorContext.h"

#if defined(jpn)
#define func_020d1198 func_020d2c64

#define data_02111304 data_02110fa4

#define data_02111728 data_021113c8
#define data_0211172c data_021113cc
#define data_02111738 data_021113d8
#define data_0211173c data_021113dc
#define data_02111754 data_021113f4
#define data_02111880 data_02111520
#define data_021118e0 data_02111580
#define data_02111f00 data_02111ba0
#endif

extern "C"
{
    CBool func_020d1198();
}

#define GET_FLAG_BIT(what, idx) (((what) & (1 << (idx))) ? 1 : 0)

struct Struct_02111728
{
    NitroHandle* firstHandle;
    NitroDirectoryAccessor romFSRoot;
};

struct Struct_0211173c
{
    unsigned int busHolderID;
    unsigned int dmaChannel;

    // For each overlay we have 0x20 bytes of metadata in the ROM.
    // The ARM9 overlays are all grouped together in a table, and the ARM7 overlays
    // are grouped together in another table. (In DQIX there are no arm7 overlays)
    // In theory, one or both of these tables could be loaded into memory,
    // in which case this would point to it. In practice, I don't think it ever is
    struct ArmOverlayDataTableData
    {
        void* start;
        unsigned int size;
    };
    ArmOverlayDataTableData arm9Data;
    ArmOverlayDataTableData arm7Data;
};

struct Struct_02111f20
{
    typedef void (*LowLevelReadProc)(Struct_02111f20*);

    LowLevelReadProc lowLevelReadProc;
    unsigned int control_4;
    unsigned int* alignedWrite;
    unsigned int unknown_C[5]; // might be padding
    // 512 = 0x200 bytes of temporary space for unaligned writes
    unsigned char scratchBuffer[512];
};

struct Struct_02111f00
{
    unsigned int number;
    unsigned int unknown_4[7];
    Struct_02111f20 innerStruct;
};

// These overlap, but the 72c is accessed from the 728 as well as on its own.
// It should be possible to just take a reference to mountedDir directly, e.g.
// if you copy assign from it you'll get .word data_02111728+0x4 (which is
// the same assembly as .word data_0211172c), but it doesn't always seem to work
extern Struct_02111728 data_02111728;
extern NitroDirectoryAccessor data_0211172c;

extern int data_02111738; // stores whether the rom fs is initialised
// Extra data for cartridge filesystem
extern Struct_0211173c data_0211173c;
// Handle for the cartridge filesystem
extern NitroHandle data_02111754;

extern Arm7CardReadData data_02111880;
extern CardReadManager data_021118e0;
extern Struct_02111f00 data_02111f00;
//extern Struct_02111f20 data_02111f20;