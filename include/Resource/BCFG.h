#pragma once

#include "../Memory/SafeAllocator.h"
#include "../Graphics/Vector.h"

// Struct/class populated using a *.bcfg file, which holds data about...
// what animations a model should use? 
class BCFG
{
public:
    // These are attached to particular animation entries, connected in a linked
    // list per animation entry. But all of these are actually in one big array
    struct SizeCEntry
    {
        unsigned short unk_0;
        unsigned short animationIndex;
        short unk_4;
        char unk_6[2];
        SizeCEntry* pNext;
    };

    // these could be the same struct
    struct AltSizeCEntry
    {
        unsigned short maybeSoundEffect;
        unsigned short animationIndex;
        fix16_t triggerTime;
        char unk_6[2];
        AltSizeCEntry* pNext;
    };

    struct AnimationRecord
    {
        char name[16];
        fix32_t startTime;
        fix32_t endTime;
        // might be fps or something more exact like that. either way, it scales
        // the speed at which animations play
        fix32_t frameRate;
        SizeCEntry* firstSizeC;
        AltSizeCEntry* firstAlt;
    };

    AnimationRecord* animations_;
    unsigned short maybeAnimationCount;
    unsigned short maybeAnimationCapacity;
    SizeCEntry* sizeCEntries_;
    unsigned short sizeCCapacity_;
    unsigned short sizeCCount_;
    
    AltSizeCEntry* altSizeCEntries_;
    unsigned short altSizeCCount_;
    unsigned short altSizeCCapacity_;
    unsigned short unknown_18;
    char unk_1a[2];
    char* maybeRootBoneName_;

    void Reset();
    void LoadFromScript(SafeAllocator* alloc, const void* data, unsigned int length);

    // ought to be private tbh
public:
    void AllocateAnimationEntries(SafeAllocator* alloc, int count);
    void InsertAnimationRecord(const AnimationRecord* entry);
    int SearchAnimationByName(const char* name);
    AnimationRecord* GetAnimationRecord(int idx);
    int GetNumAnimations() const;
    
    void AllocateSizeCEntries(SafeAllocator* alloc, int count);
    void InsertSizeCEntry(const SizeCEntry* entry);

    void AllocateAltSizeCEntries(SafeAllocator* alloc, int count);
    void InsertAltSizeCEntry(const AltSizeCEntry* entry);

    void MaybeSetRootBoneName(SafeAllocator* alloc, const char* name);
    const char* MaybeGetRootBoneName();
};