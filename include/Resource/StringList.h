#pragma once

#include <std_library_functions.h>
#include "../Memory/SafeAllocator.h"

// sizeof == 0x18
// instances exist at 02153694, 021536ac (usa)
// stores a list of strings for a particular purpose, e.g. menu strings
// or yes/no etc
class StringList
{
public:
    struct TableEntry
    {
        short index;
        const char* string;
    };

    struct NATData;

    typedef int (*Predicate)(const TableEntry*);
    typedef int (*Visitor)(NATData*, TableEntry*);
    
    struct NATData
    {
        union // this is relying on compiler-specific behavior...
        {
            uint32_t control_0_;
            struct
            {
                uint32_t numEntries_0_0_ : 12;
                uint32_t stringSectionSize_0_c_ : 19;
                uint32_t initialized_0_1f_ : 1;
            };
        };
        TableEntry* tableData_4_;
        char* stringData_8_;

        int GetNumEntries() const { return numEntries_0_0_; }
        int SetNumEntries(int to) { numEntries_0_0_ = to; }
        int GetTableSize() const;
        int GetStringSectionSize() const { return stringSectionSize_0_c_; }
        CBool IsInitialized() const { return initialized_0_1f_; }
        void MarkInitialized() { initialized_0_1f_ = true; }

        char* FixStringPointer(const char* offset, char* fallback);

        bool ForEach(Visitor proc);
        // set up tableData and stringData pointers to point into the
        // specified data (without making a copy).
        // writes the initialized bit to the provided data - definitely not const!
        bool Prepare(void* data, bool* useless, Visitor visitor);
        const TableEntry* BinarySearch(int target, Predicate pred) const;

        // takes ownership of the memory
        bool LoadFromMemory(void* data, unsigned int length);

        bool VerifySize(const void* data, unsigned int predictedLength);

        const char* GetString(int idx) const;

    } natData_0_;
    short targetIndex_c_;
    short unknown_e_;
    int taskID_10_;
    SafeAllocator* allocator_14_;    

public:
    StringList();

    void Reset();
    void Destroy();

    bool LoadFromFileEXT(SafeAllocator* alloc, const char* filename, const char* innerFilename,
        short fileNumber, short target);
    // to load from a gp2, pass (gp2, file in gp2)
    // to load directly from a nat, pass (nat, NULL)
    bool LoadFromFile(const char* filename, const char* innerFilename);
    bool StopLoad();

    bool LoadFromMemory(SafeAllocator* alloc, const void* data, unsigned int length);

    // only loads the strings whose ID is in the specified subset
    bool LoadSubsetFromMemory(SafeAllocator* alloc, void* data, unsigned int length,
        const short* subset, unsigned short subsetSize);
    
    // jpn exclusive
    // only loads the strings whose ID is in the specified subset
    bool LoadSubsetFromFile(SafeAllocator* alloc, const char* natFilename, const short* subset,
        unsigned short subsetSize, short fileNumber, bool requireLock);

    // no idea what this does, only called in one place
    bool UnknownFunction_020e03f0(void* ptr);

    // usa/eur-exclusive
    int GetNumEntries() const;
    const char* GetString(int idx) const;
    // jpn-exclusive
    char* GetAndCopyString(char* buffer, int idx) const;
};

char* GetStringFromNAT(char* buffer, void* nat, unsigned int natLength, int idx);

#if defined(jpn)

char* GetStringFromNAT(char* buffer, const char* natFilename, int idx,
    short fileNumber, bool requireLock);

#else

// fileNumber: in theory, you can put a single %d in the filename (nat / gp2 / both)
// and it will be replaced by this. In practice it doesn't ever seem to be used
char* GetStringFromNAT(char* buffer, const char* natFilename, int idx,
    short fileNumber, bool requireLock, const char* gp2Filename);

#endif