#include "World/LootableContainer.h"
#include "Resource/Script.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/FileIO.h"

#ifdef jpn
#define func_02032370 func_02031ea8

#define data_02108e78 data_02108dbc
#define data_02108e90 data_02108dd4
#endif

extern "C"
{
    // rng value between 0 and max-1 from A-table
    int func_02032370(int max);
}

struct {
    LootDistribution* distribution;
    LootableContainerManager* manager;
    SafeAllocator* allocator;
} extern data_02108e78;

extern LootableContainerManager data_02108e90;

LootableContainerManager *LootableContainerManager::GetMainInstance() {
    return &data_02108e90;
}

int Loot_Opcode_64(Script::Parameter* params, int numParams) { return 1; }

int Loot_Opcode_65(Script::Parameter* params, int numParams) { return 1; }

int LootManager_Unknown_66(Script::Parameter* params, int numParams)
{
    data_02108e78.manager->unk_10 = params[0].ToInt();
    return 1;
}

int LootManager_CreateContainer(Script::Parameter* params, int numParams)
{
    (void)func_ov017_0218b5b0();
    LootableContainerManager::Container* container =
        (LootableContainerManager::Container*)data_02108e78.allocator->Allocate(sizeof(LootableContainerManager::Container));
    if (container == NULL)
        return 0;

    unsigned int packedID = (params++)->ToInt();
    unsigned int flags = (params++)->ToInt();

    container->pNext = NULL;
    container->uniqueID = (packedID >> 16);
    container->itemIDOrRank = packedID & 0xffff;

    container->containerType = (flags >> 4) & 7;
    container->lootType = (flags >> 2) & 3;
    container->unk_4_0 = flags & 3;

    if (container->containerType == 3)
    {
        container->position.x = 0;
        container->position.y = 0;
        container->position.z = 0;
        container->unk_6 = 0;
        container->unk_6 = (params++)->ToInt();
    }
    else
    {
        if (container->containerType == 0 || container->containerType == 4)
        {
            fix32_t entries[4] = { 0 };
            for (int counter = 0; counter < 4; counter++)
            {
                if (params->type == 2)
                {
                    float asFloat = (params++)->ToFloat();
                    entries[counter] = 4096.0f * asFloat;
                }
                else if (params->type == 1)
                {
                    entries[counter] = (params++)->ToInt() * 4096;
                }
            }
            container->position.x = entries[0];
            container->position.y = entries[1];
            container->position.z = entries[2];
            container->unk_6 = entries[3];
        }
        else
        {
            fix32_t entries[3] = { 0 };
            for (int counter = 0; counter < 3; counter++)
            {
                if (params->type == 2)
                {
                    float asFloat = (params++)->ToFloat();
                    entries[counter] = 4096.0f * asFloat;
                }
                else if (params->type == 1)
                {
                    entries[counter] = (params++)->ToInt() * 4096;
                }
            }
            container->position.x = entries[0];
            container->position.y = entries[1];
            container->position.z = entries[2];
            // out of range read?! probably not used in practice
            container->unk_6 = entries[3];
        }
    }
    data_02108e78.manager->RegisterContainer(container);
    return 1;
}

void LootableContainerManager::Reset()
{
    memset(unk_0, 0, 4);
    pContainerList_ = NULL;
    numEntries_ = 0;
    unk_10 = 0;
}

void LootableContainerManager::ResetAllocator(SafeAllocator *alloc)
{
    data_02108e78.allocator = &func_ov017_0218b5b0()->lootableContainerAllocator_18c_;
    if (alloc != NULL)
        alloc->Reset();
    else
        data_02108e78.allocator->Reset();
    pContainerList_ = 0;
    numEntries_ = 0;
}

static Script::OpcodeLookupEntry s_zoneContainerOpcodes[] = {
    { 0x64, &Loot_Opcode_64 },
    { 0x65, &Loot_Opcode_65 },
    { 0x66, &LootManager_Unknown_66 },
    { 0x67, &LootManager_CreateContainer },
    { 0, NULL }
};

void LootableContainerManager::LoadZoneContainers(const void *treasureArchive,
    unsigned int archiveLength, const char *zoneName, SafeAllocator *alloc)
{
    (void)GetBattleStruct();
    char scriptFilename[20];
    sprintf(scriptFilename, "%s.bin", zoneName);
    const void* scriptFile;
    unsigned int scriptFileLength;
    if (!GetFileInNarcPermissive(treasureArchive, scriptFilename, &scriptFile, &scriptFileLength))
        return;
    data_02108e78.manager = this;
    Struct_ov017_44C8* ov17thing = func_ov017_0218b5b0();

    if (alloc != NULL)
        data_02108e78.allocator = alloc;
    else
        data_02108e78.allocator = &ov17thing->lootableContainerAllocator_18c_;
    Script runner;
    runner.Initialize();
    runner.SetOpcodeLookup(s_zoneContainerOpcodes);
    runner.Load(scriptFile, scriptFileLength);
    runner.Execute();
    data_02108e78.allocator = &ov17thing->lootableContainerAllocator_18c_;

    char allocationBuffer[0x400];
    SafeAllocator tempAlloc;
    tempAlloc.ResetAllocatorPointer(); // constructor almost certainly
    tempAlloc.CreateTypeA(allocationBuffer, sizeof(allocationBuffer));

    LootDistribution rng;
    int blankItemID = 0;

    if (GetFileInNarcPermissive(treasureArchive, "randTBox.bin", &scriptFile, &scriptFileLength))
    {
        rng.Reset();
        rng.LoadFromScript(&tempAlloc, scriptFile, scriptFileLength);
        for (Container* container = pContainerList_; container != NULL; container = container->pNext)
        {
            // blue chests only here
            if (container->containerType != 4)
                continue;
            const LootDistribution::Outcome* outcome = rng.Sample(container->itemIDOrRank);
            if (outcome != NULL)
            {
                container->lootType = outcome->lootType;
                container->itemIDOrRank = outcome->itemID;
            }
            else
            {
                container->lootType = 0;
                container->itemIDOrRank = 0;
            }
        }
        tempAlloc.Reset();
    }

    if (GetFileInNarcPermissive(treasureArchive, "randTTT.bin", &scriptFile, &scriptFileLength))
    {
        rng.Reset();
        rng.LoadFromScript(&tempAlloc, scriptFile, scriptFileLength);
        for (Container* container = pContainerList_; container != NULL; container = container->pNext)
        {
            // pots, barrels and cupboards
            if (container->containerType != 1 && container->containerType != 2 && container->containerType != 3)
                continue;
            const LootDistribution::Outcome* outcome = rng.Sample(container->itemIDOrRank);
            if (outcome != NULL)
            {
                container->lootType = outcome->lootType;
                container->itemIDOrRank = outcome->itemID;
            }
            else
            {
                container->lootType = 0;
                container->itemIDOrRank = 0;
            }
        }
        tempAlloc.Reset();
    }

    tempAlloc.Destroy(); // not the destructor, but might be called by an inline destructor?
}

void LootableContainerManager::RegisterContainer(LootableContainerManager::Container* container)
{
    LootableContainerManager::Container** pplistEnd = &pContainerList_;
    while (*pplistEnd != NULL)
        pplistEnd = &(*pplistEnd)->pNext;
    *pplistEnd = container;
    numEntries_++;
}

LootableContainerManager::Container* LootableContainerManager::GetContainerByID(int id, unsigned short *outListPos)
{
    unsigned short index = 0;
    Container* container = pContainerList_;
    while (container != NULL)
    {
        if (container->uniqueID == id)
        {
            if (outListPos != NULL)
                *outListPos = index;
            return container;
        }
        index++;
        container = container->pNext;
    }
    return NULL;
}

int LootDistribution_DeclareOutcome(Script::Parameter* params, int numParams)
{
    int packed = params[0].ToInt();
    int percentage = packed & 0x7f;
    unsigned short id = (packed >> 7) & 0xffff;
    int resultType = (packed >> 23) & 0x7;
    int rank = (packed >> 26) & 0x1f;

    LootDistribution::Outcome outcome;
    outcome.rank = rank;
    outcome.lootType = resultType;
    outcome.itemID = id;
    outcome.percentage = percentage;
    data_02108e78.distribution->InsertOutcome(&outcome);
    return 1;
}

int LootDistribution_AllocateOutcomes(Script::Parameter* params, int numParams)
{
    data_02108e78.distribution->AllocateOutcomes(params[0].ToInt());
    return 1;
}

void LootDistribution::Reset()
{
    pOutcomes_ = NULL;
    outcomeArrayCapacity_ = 0;
    outcomeArraySize_ = 0;
    pAllocator_ = NULL;
}

static Script::OpcodeLookupEntry s_distributionOpcodes[] = {
    { 0x64, &Loot_Opcode_64 },
    { 0x65, &Loot_Opcode_65 },
    { 0x69, &LootDistribution_DeclareOutcome },
    { 0x6a, &LootDistribution_AllocateOutcomes },
    { 0, NULL } 
};

void LootDistribution::LoadFromScript(SafeAllocator* alloc, const void* script, unsigned int length)
{
    data_02108e78.distribution = this;
    pAllocator_ = alloc;
    Script runner;
    runner.Initialize();
    runner.SetOpcodeLookup(s_distributionOpcodes);
    runner.Load(script, length);
    runner.Execute();
    data_02108e78.distribution = NULL;
}

void LootDistribution::InsertOutcome(Outcome *source)
{
    if (outcomeArraySize_ >= outcomeArrayCapacity_)
        return;
    int position = outcomeArraySize_++;

    Outcome* dest = &pOutcomes_[position];

    dest->rankAndLootType = source->rankAndLootType;
    dest->percentage = source->percentage;
    dest->itemID = source->itemID;
}

void LootDistribution::AllocateOutcomes(unsigned short count)
{
    pOutcomes_ = (Outcome*)pAllocator_->Allocate(count * sizeof(Outcome));
    if (pOutcomes_ != NULL)
        outcomeArrayCapacity_ = count;
}

const LootDistribution::Outcome* LootDistribution::Sample(int rank)
{
    const Outcome* outcomes[32];
    int numOutcomes = GetOutcomesByRank(rank, outcomes, 32);
    if (numOutcomes == 0)
        return NULL;

    int rng = func_02032370(100);
    for (int i = 0; i < numOutcomes; i++)
    {
        rng -= outcomes[i]->percentage;
        if (rng < 0)
            return outcomes[i];
    }

    return NULL;
}

int LootDistribution::GetOutcomesByRank(int rank, const Outcome** outList, int outCapacity)
{
    int numFound = 0;
    for (int i = 0; i < outcomeArraySize_; i++)
    {
        const Outcome* outcome = &pOutcomes_[i];
        if (outcome->rank != rank)
            continue;
        outList[numFound++] = outcome;
        if (outCapacity <= numFound)
            return numFound;
    }
    return numFound;
}