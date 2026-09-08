#pragma once

#include "../Memory/SafeAllocator.h"
#include "../Graphics/Vector.h"

// sizeof == 0xc == 12.
// Reads configurations of red/blue chests, pots, barrels etc from script files
// within data/scenario/treasure.nsarc.
class LootableContainerManager
{
public:
    struct Container
    {
        unsigned short uniqueID;
        unsigned short itemIDOrRank;
        unsigned short unk_4_0 : 2;
        // 0 = nothing, 1 = gold, 2 = item, 3 = ambush
        unsigned short lootType : 2;
        // 0 = red chest
        // 1 = pot, 2 = barrel, 3 = cupboard?
        // 4 = blue chest
        unsigned short containerType : 3;
        short unk_6;
        Vector3fix position;
        Container* pNext;
    };

    char unk_0[4];
    Container* pContainerList_;
    unsigned short numEntries_;
    short unk_10;

    // there is one main instance used by the game, but you can also create
    // a local copy just to get information - this is how Treasure Eye Land /
    // Nose for Treasure work
    static LootableContainerManager* GetMainInstance();

    LootableContainerManager() { Reset(); }
    ~LootableContainerManager() { Reset(); }

    void Reset();
    void ResetAllocator(SafeAllocator* alloc);
    void LoadZoneContainers(const void* treasureArchive, unsigned int archiveLength, const char* zoneName, SafeAllocator* alloc);
    void RegisterContainer(Container* container);
    Container* GetContainerByID(int id, unsigned short* outListPos);
};

class LootDistribution
{
public:
    struct Outcome
    {
        union {
            unsigned char rankAndLootType;
            struct {
                unsigned char rank : 5;
                // 1 = gold, 2 = regular item, 3 = ambush
                unsigned char lootType : 3;
            };
        };
        unsigned char percentage;
        unsigned short itemID; // or amount of gold, or ambush type
    };

    Outcome* pOutcomes_;
    unsigned short outcomeArrayCapacity_;
    unsigned short outcomeArraySize_;
    SafeAllocator* pAllocator_;

    void Reset();

    void LoadFromScript(SafeAllocator* alloc, const void* script, unsigned int length);
    void InsertOutcome(Outcome* atom);
    void AllocateOutcomes(unsigned short count);

    // Sample from the specified distribution
    const Outcome* Sample(int rank);
    // Returns the number put in the list
    int GetOutcomesByRank(int rank, const Outcome** outList, int outCapacity);
};