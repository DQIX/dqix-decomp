#pragma once

#include "NitroVM.h"

bool IsFileValidNarc(const unsigned char* buffer);

// sizeof(NarcHandle) == 104.
class NarcHandle
{
public:
    struct ArchiveHeader
    {
        unsigned int magic;
        unsigned short byteOrderMark;
        unsigned short version;
        unsigned int fileSize;
        unsigned short headerSize;
        unsigned short numSections;
    };

    struct SectionHeader
    {
        unsigned int magic;
        unsigned int sectionSize;
    };

    struct FATBlock
    {
        SectionHeader generic;
        unsigned short numFiles;
        unsigned short reserved;
        struct FATEntry
        {
            unsigned int startOffset;
            unsigned int endOffset;
        } entries[0];
    };
public:

    NitroHandle initial;
    const void* pNarcFile; // points to where the narc itself is loaded in memory
    const FATBlock* pFATBSection; // points to the 'FATB' header
    const void* pFileDataStart; // points past the 8-byte 'FIMG' header

    bool Initialize(const char* signatureString, const unsigned char* buffer);
    bool Destroy();

    const void* GetFileByIndex(unsigned int idx) const;
};

const void* GetFileFromNARCInMemory(const char* filename);
bool PrepareReadFileInNARCByID(NitroVM* vm, NarcHandle* handle, unsigned int id);