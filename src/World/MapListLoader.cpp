#include "World/MapListLoader.h"
#include "Resource/Script.h"
#include "Memory/SafeAllocator.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_020ef2bc data_020ef1f8
#define data_020ef2c0 data_020ef1fc

#define data_020fdc14 data_020fd980
#endif

extern int data_020ef2bc;
extern Script::OpcodeLookupEntry data_020ef2c0[];

struct ZoneGroupingList
{
    struct EntryA
    {
        char name[31];
        unsigned char count;
    };

    struct EntryB
    {
        unsigned short zoneID;
        unsigned short maybeParentZone;
        unsigned char unk_4;
        char unk_5[8];
        char unk_d[0x1c];
        union { struct {
            // 0 = fields, 1 = dungeons, 2 = indoor zones, 3 = ??? (sth with tower of nod)
            // 4 = battlefields, 5 = stornway inn, 6 = grotto, 7 = ocean/sky
            unsigned char zoneType : 3;
            unsigned char bitfield_29_bit_3 : 1;
            unsigned char bitfield_29_bit_4 : 1;
            unsigned char bitfield_29_bit_5 : 1;
            unsigned char bitfield_29_bit_6 : 1;
            unsigned char bitfield_29_bit_7 : 1;
        }; unsigned char bitfield_29; };
        unsigned char musicTrackID;
    };

    EntryA* pEntriesA;
    int capacityA;
    int countA;
    EntryB* pEntriesB;
    int unk_10;
    int capacityB;
    int unk_18;
    int unk_1c;
    SafeAllocator* allocator;

    void AllocateA(int count);
    void AllocateB(int count);

    void InsertA(const EntryA* entry);
    void InsertB(const EntryB* entry);
};

struct Struct_020fdc14
{
    int unk_0;
    MapListInfo* output;
    ZoneGroupingList* groupingInfo; // some legacy code?
} extern data_020fdc14;

extern "C"
{
}

int MapList_Opcode_64_Unused(Script::Parameter* params, int numParams) { return 1; }
int MapList_Opcode_65_Unused(Script::Parameter* params, int numParams) { return 1; }

int MapList_Opcode_66_Unused(Script::Parameter* params, int numParams)
{
    data_020fdc14.groupingInfo->AllocateB(params[0].ToInt());
    data_020fdc14.unk_0 = 0;
    return 1;
}

// doesn't match
int MapList_Opcode_67_Unused(Script::Parameter* params, int numParams)
{
    ZoneGroupingList::EntryB entry;
    
    entry.zoneID = 0;
    entry.zoneType = 0;
    entry.maybeParentZone = 0;
    entry.bitfield_29_bit_4 = true;
    entry.bitfield_29_bit_3 = false;
    entry.bitfield_29_bit_5 = false;
    entry.bitfield_29_bit_6 = false;
    entry.unk_5[0] = '\0';
    entry.unk_4 = 0;
    entry.bitfield_29_bit_7 = false;
    entry.zoneID = (params++)->ToInt();
    entry.maybeParentZone = (params++)->ToInt();
    (void)(params++)->ToString();
    entry.zoneType = (params++)->ToInt();
    const char* shortName = (params++)->ToString();
    const char* longName = (params++)->ToString();
    entry.musicTrackID = (params++)->ToInt();
    entry.bitfield_29_bit_4 = (params++)->ToInt();
    entry.bitfield_29_bit_3 = (params++)->ToInt();
    entry.bitfield_29_bit_5 = (params++)->ToInt();
    entry.bitfield_29_bit_6 = (params++)->ToInt();
    entry.bitfield_29_bit_7 = (params++)->ToInt(); // 12th argument is usually a string..?

    if (longName == NULL)
        return 0;
      
    memcpy(entry.unk_5, shortName, 8);
    entry.unk_5[7] = '\0';
    
    memcpy(entry.unk_d, longName, 0x1c);
    
    entry.unk_d[0x1b] = '\0';
    Struct_020fdc14* info = &data_020fdc14;
    info++;
    (info - 1)->groupingInfo->InsertB(&entry);
    return 1;
}

int MapList_Opcode_68_Unused(Script::Parameter* params, int numParams)
{
    const char* name = params[0].ToString();
    ZoneGroupingList::EntryA entry;
    memset(entry.name, 0, 31);
    strcpy(entry.name, name);
    data_020fdc14.groupingInfo->InsertA(&entry);
    return 1;
}

int MapList_Opcode_69_Unused(Script::Parameter* params, int numParams)
{
    data_020fdc14.groupingInfo->AllocateA(params[0].ToInt());
    return 1;
}

void ZoneGroupingList::AllocateA(int count)
{
    pEntriesA = (EntryA*)allocator->Allocate(count * sizeof(EntryA));
    if (pEntriesA != NULL)
    {
        capacityA = count;
        countA = 0;
    }
}

void ZoneGroupingList::AllocateB(int count)
{
    pEntriesB = (EntryB*)allocator->Allocate(count * sizeof(EntryB));
    if (pEntriesB != NULL)
        capacityB = count;
}

void ZoneGroupingList::InsertA(const EntryA* source)
{
    if (pEntriesA != NULL)
    {
        if (countA >= capacityA)
            return;

        EntryA* dest = &pEntriesA[countA];
        *dest = *source;
        countA++;
    }
    else
        countA++;
}


void ZoneGroupingList::InsertB(const EntryB* source)
{
    if (pEntriesA != NULL && countA < capacityA)
    {
        pEntriesA[countA - 1].count++;
    }
    if (unk_18 >= 0 && unk_18 != countA - 1)
        return;
    if (unk_1c >= 0 && unk_1c != source->zoneType)
        return;

    if (pEntriesB == NULL)
        return;
    if (unk_10 >= capacityB)
        return;
    
    EntryB* dest = &pEntriesB[unk_10];
    // this should just be struct copy assignment
    dest->zoneID = source->zoneID;
    dest->maybeParentZone = source->maybeParentZone;
    dest->unk_4 = source->unk_4;
    COPY_ARRAY(dest->unk_5, source->unk_5);
    COPY_ARRAY(dest->unk_d, source->unk_d);
    dest->bitfield_29 = source->bitfield_29;
    dest->musicTrackID = source->musicTrackID;
    if (countA > 0)
        pEntriesB[unk_10].unk_4 = countA - 1;
    unk_10++;
}

int MapList_Opcode_66(Script::Parameter* params, int numParams) { return 1; }

int MapList_Opcode_67(Script::Parameter* params, int numParams)
{
    if (data_020ef2bc != (params++)->ToInt())
        return 1;

    (void)(params++)->ToInt();
    const char* maybeDisplayName = (params++)->ToString();
#if defined(jpn)
    strcpy(data_020fdc14.output->jp_buffer, maybeDisplayName);
#endif
    (void)(params++)->ToInt();
    (void)(params++)->ToString();
    (void)(params++)->ToString();
    (void)(params++)->ToInt();
    (void)(params++)->ToString();
    (void)(params++)->ToInt();
    (void)(params++)->ToInt();
    (void)(params++)->ToInt();

    const char* modelName = (params++)->ToString();
    if (modelName != NULL)
        strcpy(data_020fdc14.output->maybeModelName, modelName);
    (void)(params++)->ToInt();
    float zoneRotation = (params++)->ToFloat();
    data_020fdc14.output->worldRotation = fix32ReduceAngle0To2Pi(4096.0f * zoneRotation);
    data_020fdc14.output->unknown_38 = 4096.0f * (params++)->ToFloat();
    data_020fdc14.output->unknown_3c = 4096.0f * (params++)->ToFloat();
    return 1;
}

int MapList_Opcode_68(Script::Parameter* params, int numParams) { return 1; }
int MapList_Opcode_69(Script::Parameter* params, int numParams) { return 1; }

void LoadZoneInfoFromMapListScript(int zoneID, MapListInfo* outInfo, const void* script, unsigned int length)
{
    data_020fdc14.output = outInfo;
    data_020ef2bc = zoneID;
    Script runner;
    runner.Initialize();
    runner.SetOpcodeLookup(data_020ef2c0);
    runner.Load(script, length);
    runner.Execute();
    data_020fdc14.output = NULL;
    data_020ef2bc = -1;
}