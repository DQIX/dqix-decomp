#include "World/BMDJ.h"
#include "Resource/Script.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"

extern Script::OpcodeLookupEntry data_020ef418[];

struct Struct_020fdc40
{
    Zone3D_BMDJStruct::StructSizeC* structC;
    SafeAllocator* allocator;
    Zone3D_BMDJStruct::ScriptData* instance;
} extern data_020fdc40;

int BMDJ_Opcode_6a(Script::Parameter* params, int numParams)
{
    int count = params[0].ToInt();
    Zone3D_BMDJStruct::ScriptData* instance = data_020fdc40.instance;
    instance->ptr_0 = (Zone3D_BMDJStruct::StructSizeC*)data_020fdc40.allocator->Allocate(count * sizeof(Zone3D_BMDJStruct::StructSizeC));
    instance->capacity_8 = count;
    return 1;
}

int BMDJ_Opcode_6b(Script::Parameter* params, int numParams) { return 1; }

int BMDJ_Opcode_6c(Script::Parameter* params, int numParams)
{
    Zone3D_BMDJStruct::StructSizeC entry;
    entry.maybeID = (params++)->ToInt();
    const char* name = (params++)->ToString();
    if (name == NULL)
        return 0;
    entry.name = (char*)data_020fdc40.allocator->Allocate(strlen(name) + 1);
    strcpy(entry.name, name);
    entry.unk_2 = 0;
    if (numParams >= 3)
    {
        entry.unk_2 = (params++)->ToInt();
        entry.unk_2 &= 0x3f;
    }
    entry.unk_3 = 0;
    
    if (numParams >= 4)
    {
        entry.unk_3 = (params++)->ToInt();
    }

    entry.pNext = NULL;

    data_020fdc40.instance->InsertStructC(entry);
    data_020fdc40.structC = data_020fdc40.instance->GetStructCByID(entry.maybeID);
    return 1;
}

int BMDJ_Opcode_72(Script::Parameter* params, int numParams)
{
    if (data_020fdc40.structC == NULL)
        return 0;

    Zone3D_BMDJStruct::StructSizeC* newEntry = (Zone3D_BMDJStruct::StructSizeC*)data_020fdc40.allocator->Allocate(sizeof(Zone3D_BMDJStruct::StructSizeC));
    newEntry->maybeID = (params++)->ToInt();
    const char* name = (params++)->ToString();
    if (name == NULL)
        return 0;
    newEntry->name = (char*)data_020fdc40.allocator->Allocate(strlen(name) + 1);
    strcpy(newEntry->name, name);
    newEntry->unk_2 = 0;
    if (numParams >= 3)
    {
        newEntry->unk_2 = (params++)->ToInt();
        newEntry->unk_2 &= 0x1f;
    }
    newEntry->unk_3 = 0;
    if (numParams >= 4)
        newEntry->unk_3 = (params++)->ToInt();
    newEntry->pNext = NULL;

    Zone3D_BMDJStruct::StructSizeC* listEnd = data_020fdc40.structC;
    while (listEnd->pNext != NULL)
        listEnd = listEnd->pNext;

    listEnd->pNext = newEntry;
    return 1;
}

int BMDJ_Opcode_6d(Script::Parameter* params, int numParams)
{
    data_020fdc40.instance->AllocateStruct20Array(params[0].ToInt(), data_020fdc40.allocator);
    return 1;
}

int BMDJ_Opcode_6e(Script::Parameter* params, int numParams) { return 1; }

int BMDJ_Opcode_6f(Script::Parameter* params, int numParams)
{
    int id = (params++)->ToInt();
    int arg1 = (params++)->ToInt();
    float tX = (params++)->ToFloat();
    float tY = (params++)->ToFloat();
    float tZ = (params++)->ToFloat();
    int arg5 = (params++)->ToInt();
    (void)(params++)->ToString();

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;
    if (numParams > 10)
    {
        scaleX = (params++)->ToFloat();
        scaleY = (params++)->ToFloat();
        scaleZ = (params++)->ToFloat();
    }

    float x3 = 0.0f;
    float y3 = 0.0f;
    float z3 = 0.0f;
    if (numParams > 7)
    {
        x3 = (params++)->ToFloat();
        y3 = (params++)->ToFloat();
        z3 = (params++)->ToFloat();
    }

    unsigned char flags = 0xf;
    if (numParams > 13)
    {
        flags = (params++)->ToInt();
        if (flags & 0x10)
            flags &= ~0x10;
        else
            flags |= 0x10;
        if (flags & 0x20)
            flags &= ~0x20;
        else
            flags |= 0x20;
    }

    Zone3D_BMDJStruct::StructSize20 entry;
    entry.maybeID = id;
    entry.unk_2 = arg1;
    entry.unk_4 = arg5;
    entry.unk_8[0] = 4096.0f * tX;
    entry.unk_8[1] = 4096.0f * tY;
    entry.unk_8[2] = 4096.0f * tZ;

    entry.unk_14 = (fix32_t)(4096.0f * x3);
    entry.unk_16 = (fix32_t)(4096.0f * y3);
    entry.unk_18 = (fix32_t)(4096.0f * z3);

    entry.unk_1a = (fix32_t)(4096.0f * scaleX);
    entry.unk_1c = (fix32_t)(4096.0f * scaleY);
    entry.unk_1e = (fix32_t)(4096.0f * scaleZ);

    Zone3D_BMDJStruct::ScriptData* instance = data_020fdc40.instance;
    instance++; // silly hack for matching assembly
    entry.flags_6 = flags;
    (instance - 1)->InsertStruct20(entry);
    return 1;
}

int BMDJ_Opcode_70(Script::Parameter* params, int numParams)
{
    const char* name = params[0].ToString();
    if (name != NULL)
        data_020fdc40.instance->StoreString(name, data_020fdc40.allocator);
    return 1;
}

void Zone3D_BMDJStruct::ScriptData::Reset()
{
    ptr_0 = NULL;
    counter_4 = 0;
    capacity_8 = 0;
    ptr_c = NULL;
    counter_10 = 0;
    capacity_14 = 0;
    stringCount_38 = 0;
}

void Zone3D_BMDJStruct::ScriptData::Load(SafeAllocator *alloc, const void *script, unsigned int length)
{
    bool externalFile = false;
    unsigned int localLength = length;
    if (script == NULL)
    {
        BackgroundLoader::AddLockGlobal();
        externalFile = true;
        script = LoadFileIntoMemory(NULL, data_0211e33c, &localLength);
    }
    if (script != NULL)
    {
        data_020fdc40.allocator = alloc;
        data_020fdc40.instance = this;

        Script runner;
        runner.Initialize();
        runner.SetOpcodeLookup(data_020ef418);
        runner.Load(script, localLength);
        runner.Execute();

        data_020fdc40.instance = NULL;
        data_020fdc40.allocator = NULL;
    }

    if (externalFile)
        BackgroundLoader::RemoveLockGlobal();
}

void Zone3D_BMDJStruct::ScriptData::InsertStructC(const StructSizeC& source)
{
    if (capacity_8 <= counter_4)
        return;
    StructSizeC& dest = ptr_0[counter_4++];
    dest.maybeID = (unsigned short)source.maybeID;
    dest.unk_2 = source.unk_2;
    dest.unk_3 = source.unk_3;
    dest.name = source.name;
    dest.pNext = source.pNext;
}

Zone3D_BMDJStruct::StructSizeC* Zone3D_BMDJStruct::ScriptData::GetStructC(int idx)
{
    if (idx < 0 || capacity_8 <= idx) // bug? should be size and not capacity
        return NULL;
    return &ptr_0[idx];
}

Zone3D_BMDJStruct::StructSizeC* Zone3D_BMDJStruct::ScriptData::GetStructCByID(int id)
{
    for (int i = 0; i < counter_4; i++)
    {
        if (id == ptr_0[i].maybeID)
            return &ptr_0[i];
    }
    return NULL;
}

int Zone3D_BMDJStruct::ScriptData::FindStructCByID(int id) const
{
    for (int i = 0; i < counter_4; i++)
    {
        if (id == ptr_0[i].maybeID)
            return i;
    }
    return -1;
}

void Zone3D_BMDJStruct::ScriptData::AllocateStruct20Array(int count, SafeAllocator* alloc)
{
    ptr_c = (StructSize20*)alloc->Allocate(count * sizeof(StructSize20));
    capacity_14 = count;
}

void Zone3D_BMDJStruct::ScriptData::InsertStruct20(const StructSize20& source)
{
    if (capacity_14 <= counter_10)
        return;
    StructSize20& dest = ptr_c[counter_10++];
    dest.maybeID = source.maybeID;
    dest.unk_2 = source.unk_2;
    dest.unk_4 = source.unk_4;
    dest.flags_6 = source.flags_6;
    COPY_ARRAY(dest.unk_8, source.unk_8);
    dest.unk_14 = (unsigned short)source.unk_14;
    dest.unk_16 = (unsigned short)source.unk_16;
    dest.unk_18 = (unsigned short)source.unk_18;
    dest.unk_1a = (unsigned short)source.unk_1a;
    dest.unk_1c = (unsigned short)source.unk_1c;
    dest.unk_1e = (unsigned short)source.unk_1e;
}

Zone3D_BMDJStruct::StructSize20* Zone3D_BMDJStruct::ScriptData::GetStruct20(int idx)
{
    if (idx < 0 || counter_10 <= idx)
        return NULL;
    return &ptr_c[idx];
}

Zone3D_BMDJStruct::StructSize20* Zone3D_BMDJStruct::ScriptData::GetStruct20ByID(int id)
{
    for (int i = 0; i < counter_10; i++)
    {
        if (id == ptr_c[i].maybeID)
            return &ptr_c[i];
    }
    return NULL;
}

void Zone3D_BMDJStruct::ScriptData::StoreString(const char* str, SafeAllocator* alloc)
{
    char* copy = (char*)alloc->Allocate(strlen(str) + 1);
    strings_18[stringCount_38] = copy;
    if (strings_18[stringCount_38] == NULL)
        return;
    strcpy(strings_18[stringCount_38], str);
    stringCount_38++;
}

const char* Zone3D_BMDJStruct::ScriptData::GetString(int idx) const
{
    if (idx < 0 || stringCount_38 <= idx)
        return NULL;
    return strings_18[idx];
}