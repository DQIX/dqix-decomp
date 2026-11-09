#pragma ipa file
#include "Resource/StringList.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include <globaldefs.h>
#include <asmhacks.h>

#if defined(jpn)
#define func_02072aa8 func_02073c38
#define func_020da150 func_020dbb58
#endif

extern "C"
{
    void func_02072aa8(void*, void*, int);
    // allocate & copy string
    char* func_020da150(SafeAllocator*, const char*);
}

#if defined(jpn)
void* GetNAT(const char* natFilename, unsigned int* outLength, short unused);
bool CopyAndReplaceFilename(char* dst, const char* src, short unused);
#else
void* GetNAT(const char* natFilename, unsigned int* outLength, short unused, const char* gp2Filename);
bool CopyAndReplaceFilename(char* dst, const char* src, short unused);
#endif

static int IndexPredicate(const StringList::TableEntry* entry)
{
    return entry->index;
}

static int ConvertOffsetsToPointers(StringList::NATData* nat, StringList::TableEntry* entry)
{
    entry->string = nat->FixStringPointer(entry->string, NULL);
    return true;
}

char* StringList::NATData::FixStringPointer(const char* initial, char* fallback)
{
    char* ret = fallback;
    bool useFallback = true;
    int offset = initial - (char*)NULL;
    
    if (offset != -1 && stringData_8_ != NULL)
        useFallback = false;

    if (!useFallback)
        ret = stringData_8_ + offset;
    return ret;
}

StringList::StringList()
{
    Reset();
}

void StringList::Reset()
{
    memset(&natData_0_, 0, sizeof(NATData));
    targetIndex_c_ = -1;
    taskID_10_ = -1;
    StopLoad();
}

void StringList::Destroy()
{
    StopLoad();
    Reset();
}

bool StringList::LoadFromFileEXT(SafeAllocator* alloc, const char* filename,
    const char* innerFilename, short fileNumber, short target)
{
    StopLoad();
    Reset();
    allocator_14_ = alloc;
    targetIndex_c_ = target;
    unknown_e_ = 1;
    if (fileNumber >= 0)
    {
        char replacedFilename[64];
        CopyAndReplaceFilename(replacedFilename, filename, fileNumber);
        if (innerFilename != NULL)
        {
            char replacedInnerName[32];
            CopyAndReplaceFilename(replacedInnerName, innerFilename, fileNumber);
            LoadFromFile(replacedFilename, replacedInnerName);
        }
        else
            LoadFromFile(replacedFilename, NULL);
    }
    else
    {
        if (innerFilename != NULL)
            LoadFromFile(filename, innerFilename);
        else
            LoadFromFile(filename, NULL);
    }
}

bool StringList::LoadFromFile(const char* filename, const char* innerFilename)
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    switch (unknown_e_)
    {
    case 1:
        if (filename == NULL)
        {
            StopLoad();
            return true;
        }
        taskID_10_ = (innerFilename != NULL) ?
            loader->QueueLoadFileInGP2(filename, innerFilename, NULL) :
            loader->QueueLoadFile(filename, NULL);
        unknown_e_ = 2;
        return LoadFromFile(NULL, NULL);
    case 2:
        if (loader->GetTaskStatus(taskID_10_) == 0)
            return false;
        unknown_e_ = 3;
        return LoadFromFile(NULL, NULL);
    case 3:
    {
        void* fileData = NULL;
        unsigned int fileSize = 0;
        loader->GetLoadedFileByID(taskID_10_, &fileData, &fileSize);
        if (targetIndex_c_ < 0)
        {
            LoadFromMemory(allocator_14_, fileData, fileSize);
        }
        else
        {
            short searchIndex = targetIndex_c_;
            LoadSubsetFromMemory(allocator_14_, fileData, fileSize, &searchIndex, 1);
        }
        StopLoad();
        return true;
    }
    }
    return true;
}

bool StringList::StopLoad()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (taskID_10_ >= 0)
    {
        loader->RemoveTask(taskID_10_);
        taskID_10_ = -1;
    }
    unknown_e_ = 0;
    allocator_14_ = NULL;
    return true;
}

bool StringList::LoadFromMemory(SafeAllocator* alloc, const void* data, unsigned int length)
{
    if (alloc == NULL || data == NULL || length == 0)
        return false;

    if (alloc == NULL || data == NULL) // ...
        return false;

    memcpy(&natData_0_.control_0_, data, 4);
    int tableSize = natData_0_.GetTableSize();
    int bufferSize = natData_0_.GetStringSectionSize();

    if (tableSize != 0)
        natData_0_.tableData_4_ = (TableEntry*)alloc->Allocate(tableSize);
    else
        natData_0_.tableData_4_ = NULL;

    if (bufferSize != 0)
        natData_0_.stringData_8_ = (char*)alloc->Allocate(bufferSize);
    else 
        natData_0_.stringData_8_ = NULL;

    if (natData_0_.tableData_4_ != NULL)
        memcpy(natData_0_.tableData_4_, (const char*)data + 4, tableSize);

    if (natData_0_.stringData_8_ != NULL)
        memcpy(natData_0_.stringData_8_, (const char*)data + (natData_0_.GetTableSize() + 4), bufferSize);
    
    // cast ensures the null check isn't optimized out
    Visitor fixPtrs = (Visitor)&ConvertOffsetsToPointers;
    if (fixPtrs != NULL)
        natData_0_.ForEach(fixPtrs);

    natData_0_.MarkInitialized();
    return true;
}

int StringList::NATData::GetTableSize() const
{
    if (ALWAYS_FALSE())
        return 0;
    return numEntries_0_0_ * 8;
}

bool StringList::NATData::ForEach(Visitor proc)
{
    if (tableData_4_ == NULL || GetNumEntries() == 0 || proc == NULL)
        return false;
    int numEntries = GetNumEntries();
    TableEntry* loopEntry = tableData_4_;
    for (int i = 0; i < numEntries; i++, loopEntry++)
    {
        proc(this, loopEntry);
    }
    return true;
}

static inline void CopyEntry(StringList::TableEntry* dst, StringList::TableEntry* src)
{
    dst->index = src->index;
    dst->string = src->string;
}

static inline void SwapEntries(StringList::TableEntry* a, StringList::TableEntry* b)
{
    StringList::TableEntry tmp = *a;
    CopyEntry(a, b);
    CopyEntry(b, &tmp);
}

bool StringList::LoadSubsetFromMemory(SafeAllocator* alloc, void* data,
    unsigned int length, const short* validIndices, unsigned short numValidIndices)
{
    if (alloc == NULL || data == NULL || length == 0 || validIndices == NULL || numValidIndices == 0)
        return false;
    memset(&natData_0_, 0, sizeof(NATData));
    
    NATData natCopy;
    bool useless;
    memset(&natCopy, 0, sizeof(NATData));
    natCopy.Prepare(data, &useless, NULL);

    const short* pSearchTarget = validIndices;
    int numMatching = 0;
    TableEntry* writeDst;
    int searchIdx;
    int searchEnd = numValidIndices;
    for (searchIdx = 0; searchIdx < searchEnd; searchIdx++, pSearchTarget++)
    {
        if (natCopy.BinarySearch(*pSearchTarget, &IndexPredicate) != NULL)
            numMatching++;
    }

    natData_0_.tableData_4_ = (TableEntry*)alloc->Allocate(numMatching * sizeof(TableEntry));
    if (natData_0_.tableData_4_ != NULL)
    {
        natData_0_.SetNumEntries(numMatching);
        pSearchTarget = validIndices;
        writeDst = natData_0_.tableData_4_;
        for (searchIdx = 0; searchIdx < searchEnd; searchIdx++, pSearchTarget++)
        {
            const TableEntry* found = natCopy.BinarySearch(*pSearchTarget, &IndexPredicate);
            if (found == NULL)
                continue;
            const char* name = natCopy.FixStringPointer(found->string, NULL);
            writeDst->index = found->index;
            writeDst->string = func_020da150(alloc, name);
            writeDst++;
        }
    }

    // cocktail shaker sort?!
    if (numMatching >= 2 && this != NULL)
    {
        int numEntries = natData_0_.GetNumEntries();
        if (natData_0_.tableData_4_ != NULL && numEntries != 0)
        {
            int fwdIndex;
            int backIndex;
            int upperLimit = numEntries - 1;
            int lowerLimit = 0;
            while (true)
            {
                backIndex = -1;
                TableEntry* sortCursor = natData_0_.tableData_4_ + lowerLimit;
                for (fwdIndex = lowerLimit; fwdIndex < upperLimit; sortCursor++, fwdIndex++)
                {
                    if (sortCursor[0].index > sortCursor[1].index)
                    {
                        TableEntry* otherEntry = sortCursor + 1;
                        backIndex = fwdIndex;
                        SwapEntries(sortCursor, otherEntry);
                    }
                }
                if (backIndex == -1)
                    break;
                upperLimit = backIndex;
                TableEntry* nextEntry;
                TableEntry* currentEntry = natData_0_.tableData_4_ + upperLimit;
                int lastBackSwap = -1;
                while (backIndex > lowerLimit)
                {
                    nextEntry = currentEntry - 1;
                    if (nextEntry->index > currentEntry->index)
                    {
                        lastBackSwap = backIndex;
                        TableEntry temp = *currentEntry;
                        CopyEntry(currentEntry, nextEntry);
                        CopyEntry(nextEntry, &temp);
                    }
                    currentEntry = nextEntry;
                    backIndex--;
                }
                if (lastBackSwap == -1)
                    break;
                lowerLimit = lastBackSwap;
            }
        }
    }

    return true;
}

bool StringList::NATData::Prepare(void* data, bool* useless, Visitor visitor)
{
    *useless = false;
    if (data == NULL)
        return false;
    memcpy(&control_0_, data, 4);
    tableData_4_ = (TableEntry*)((char*)data + 4);
    int stringOffset = GetTableSize() + 4;
    stringData_8_ = (char*)data + stringOffset;
    if (IsInitialized())
    {
        *useless = true;
        return true;
    }
    ForEach(visitor);
    MarkInitialized();
    ((NATData*)data)->MarkInitialized();
    return true;
}

const StringList::TableEntry* StringList::NATData::BinarySearch(int target, Predicate predicate) const
{
    TableEntry* entries = tableData_4_;

    if (entries == NULL || predicate == NULL)
        return NULL;
    if (GetNumEntries() == 0)
        return NULL;
    
    int searchMin = 0;
    int searchMax = GetNumEntries() - 1;
    while (searchMin <= searchMax)
    {
        int candidateIndex = searchMin + ((searchMax - searchMin + 1) >> 1);
        TableEntry* candidateEntry = &entries[candidateIndex];
        int outcome = predicate(candidateEntry);
        if (outcome == target)
            return candidateEntry;
        else if (outcome > target)
            searchMax = candidateIndex - 1;
        else
            searchMin = candidateIndex + 1;
    }

    return NULL;
}

bool StringList::NATData::LoadFromMemory(void* data, unsigned int length)
{
    if (data == NULL || length == 0)
        return false;
    bool useless;
    Prepare(data, &useless, &ConvertOffsetsToPointers);
    return true;
}

bool StringList::NATData::VerifySize(const void* argData, unsigned int predictedLength)
{
    NATData* data = (NATData*)argData;
    if (data == NULL || predictedLength == 0)
        return false;
    unsigned int trueLength = 4 + data->GetNumEntries() * 8 + data->GetStringSectionSize();
    return trueLength == predictedLength;
}

#ifdef jpn
bool StringList::LoadSubsetFromFile(SafeAllocator* alloc, const char* natFilename,
    const short* searchIndices, unsigned short numSearchIndices, short fileNumber, bool requireLock)
{
    if (requireLock)
        BackgroundLoader::AddLockGlobal();
    
    unsigned int natLength;
    void* nat = GetNAT(natFilename, &natLength, fileNumber);
    bool success = LoadSubsetFromMemory(alloc, nat, natLength, searchIndices, numSearchIndices);

    if (requireLock)
        BackgroundLoader::RemoveLockGlobal();
    return success;
}
#endif

bool StringList::UnknownFunction_020e03f0(void* ptr)
{
    if (ptr == NULL)
        return false;
    func_02072aa8(ptr, natData_0_.tableData_4_, GetNumEntries());
    return true;
}

int StringList::GetNumEntries() const
{
    return natData_0_.GetNumEntries();
}

const char* StringList::GetString(int idx) const
{
    return natData_0_.GetString(idx);
}

char* StringList::GetAndCopyString(char* buffer, int idx) const
{
    if (buffer == NULL)
        return NULL;
    const char* src = natData_0_.GetString(idx);
    if (src == NULL)
    {
        *buffer = '\0';
        return NULL;
    }
    strcpy(buffer, src);
    return buffer;
}

const char* StringList::NATData::GetString(int idx) const
{
    if (this == NULL)
        return NULL;
    const TableEntry* result = BinarySearch(idx, &IndexPredicate);
    if (result != NULL)
        return result->string;
    return NULL;
}

char* GetStringFromNAT(char* buffer, void* nat, unsigned int natLength, int idx)
{
    if (buffer == NULL || nat == NULL || natLength == 0 || idx < 0)
        return NULL;
    StringList::NATData natHandle;
    memset(&natHandle, 0, sizeof(StringList::NATData));
    bool useless;
    natHandle.Prepare(nat, &useless, &ConvertOffsetsToPointers);
    const char* result = natHandle.GetString(idx);
    if (result == NULL)
    {
        *buffer = '\0'; // write out empty string
        return NULL;
    }
    strcpy(buffer, result);
    return buffer;
}

char* GetStringFromNAT(char* buffer, const char* natFilename, int idx,
    short number, bool requireLock
#if !defined(jpn)
    , const char* gp2Filename
#endif
){
    if (buffer == NULL || natFilename == NULL || idx < 0)
        return NULL;
    if (requireLock)
        BackgroundLoader::AddLockGlobal();
    unsigned int natLength;
#if defined(jpn)
    GetNAT(natFilename, &natLength, number);
    char* result = GetStringFromNAT(buffer, data_0211e33c, natLength, idx);
#else
    void* nat = GetNAT(natFilename, &natLength, number, gp2Filename);
    char* result = GetStringFromNAT(buffer, nat, natLength, idx);
#endif
    if (requireLock)
        BackgroundLoader::RemoveLockGlobal();
    return result;
}

void* GetNAT(const char* natFilename, unsigned int* outLength, short number
#if !defined(jpn)
    , const char* gp2Filename
#endif
){
    if (natFilename == NULL)
        return NULL;
    *outLength = 0;
    char replacedNatName[64];
    CopyAndReplaceFilename(replacedNatName, natFilename, number);
#if !defined(jpn)
    if (gp2Filename == NULL)
    {
#endif
        LoadFileIntoMemory(replacedNatName, data_0211e33c, outLength);
        if (*outLength != 0)
            return data_0211e33c;
#if !defined(jpn)
    }
    else
    {
        void* extraction = ExtractFileFromGP2(gp2Filename, replacedNatName, outLength);
        if (extraction != NULL)
            return extraction;
    }
#endif
    return NULL;
}

bool CopyAndReplaceFilename(char* dst, const char* src, short number)
{
    if (dst == NULL || src == NULL)
        return false;
    
    sprintf(dst, src, number);
    char* extension = strstr(dst, ".bin");
    if (extension != NULL)
        strcpy(extension, ".nat");
    return true;
}