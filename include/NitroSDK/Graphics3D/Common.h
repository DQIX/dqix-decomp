#pragma once

#include <std_library_functions.h>
#include <globaldefs.h>
#include "../Matrix.h"

struct Nitro3DFileHeader
{
    uint32_t signature;
    uint16_t byteOrderMark;
    uint16_t version;
    uint32_t fileSize;
    uint16_t headerSize;
    uint16_t numSubfiles;
};

struct Nitro3DNameList
{
    uint8_t unknown_0;
    uint8_t numEntries;
    uint16_t totalSize; // size in bytes of the entire list

    // subheader?
    uint16_t subheaderSize;
    uint16_t offsetToDataStart;

    struct SearchTreeEntry // https://github.com/scurest/nsbmd_docs/issues/2
    {
        uint8_t bitIndex;
        uint8_t children[2];
        uint8_t resourceIndex;
    } treeRoot;
    // Supposedly the structure continues as follows:
    // SearchTreeEntry otherEntries_[numEntries_];
    // <-- this + offsetToDataStart_ points to the following:
    // uint16_t elementStride_; // holds sizeof(T)
    // uint16_t dataSectionSize_; // includes elementStride and dataSectionSize, so is equal to numEntries * sizeof(T) + 4
    // T dataElements_[numEntries_];
    // Name names_[numEntries_]; // Name is an array of 16 chars

    template<class T>
    inline T* GetEntryByIndex(unsigned int n) const
    {
        if (this != NULL && n < numEntries)
        {
            const char* dataStart = (const char*)this + offsetToDataStart;
            uint16_t stride = *(uint16_t*)dataStart;
            return (T*)(dataStart + 4 + stride * n);
        }
        return NULL;
    }

    template<class T>
    inline T* GetEntryFromu32Offset(unsigned int n) const
    {
        if (this != NULL)
        {
            uint32_t* pOffset = GetEntryByIndex<uint32_t>(n);
            if (pOffset != NULL)
                return (T*)((const char*)this + *pOffset);
        }
        return NULL;
    }

    inline const char* GetNameByIndex(unsigned int n) const
    {
        const uint16_t* dataStart;
        if (this != NULL && n < numEntries)
        {
            dataStart = (const uint16_t*)((const char*)this + offsetToDataStart);
            return (const char*)((const char*)dataStart + dataStart[1] + 16 * n);
        }
        else
            return NULL;
    }
};

struct Nitro3DInnerFileHeader
{
    uint32_t signature;
    uint32_t fileSize;
    Nitro3DNameList nameList;
};
