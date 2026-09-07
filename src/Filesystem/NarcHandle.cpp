#include "Filesystem/NarcHandle.h"
#include "std_library_functions.h"
#include "Filesystem/NitroVM.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileAccessor.h"
#include <globaldefs.h>

#define NARC_HEADER 0x4352414e
#define BYTE_ORDER_MARK 0xfffe

#define SIGNATURE_FATB 0x46415442
#define SIGNATURE_FIMG 0x46494d47
#define SIGNATURE_FNTB 0x464e5442

#pragma optimize_for_size off

bool IsFileValidNarc(const unsigned char* buffer)
{
    const NarcHandle::ArchiveHeader& narc = *(const NarcHandle::ArchiveHeader*)buffer;
    
    if (narc.magic != NARC_HEADER)
        return false;

    if (narc.byteOrderMark != BYTE_ORDER_MARK)
        return false;

    if (narc.version != 0x0100)
        return false;
    return true;
}

bool NarcHandle::Initialize(const char* signatureString, const unsigned char* buffer)
{
    const SectionHeader* fatPtr = NULL;
    const SectionHeader* fntPtr = NULL;
    const SectionHeader* imgPtr = NULL;

    if (!IsFileValidNarc(buffer))
        return false;

    const ArchiveHeader& header = *(const ArchiveHeader*)buffer;
    int i = 0;
    const SectionHeader* sectionPtr = (const SectionHeader*)(buffer + header.headerSize);

    if ((int)header.numSections > 0)
    {
        do
        {
            switch (sectionPtr->magic)
            {
            case SIGNATURE_FATB:
                fatPtr = sectionPtr;
                break;
            case SIGNATURE_FIMG:
                imgPtr = sectionPtr;
                break;
            case SIGNATURE_FNTB:
                fntPtr = sectionPtr;
                break;
            }                

            i++;
            sectionPtr = (const SectionHeader*)((const char*)sectionPtr + sectionPtr->sectionSize);
        } while (i < header.numSections);
        
    }

    NitroHandle_Initialize(&initial);
    pNarcFile = buffer;
    pFATBSection = (const FATBlock*)fatPtr;
    unsigned char* imgPostHeader = (unsigned char*)(imgPtr) + 8;
    pFileDataStart = imgPostHeader;

    if (!NitroHandle_AddToHandleList(&initial, signatureString, strlen(signatureString)))
        return false;

    if (NitroHandle_Populate(&initial, imgPostHeader,
            (unsigned int)fatPtr + 12 - (unsigned int)imgPostHeader, fatPtr->sectionSize - 12,
            (unsigned int)fntPtr + 8 - (unsigned int)imgPostHeader, fntPtr->sectionSize - 8,
            NULL, NULL)) // use default (memcpy-based) read and write procs
        return true;
    else
    {
        NitroHandle_RemoveFromHandleList(&initial);
        return false;
    }
}

bool NarcHandle::Destroy()
{
    if (!NitroHandle_Destroy(&initial))
        return false;

    NitroHandle_RemoveFromHandleList(&initial);
    return true;
}

const void* GetFileFromNARCInMemory(const char* filename)
{
    const unsigned char* addr = NULL;
    NitroVM machine;
    NitroVM_Initialize(&machine);

    if (NitroVM_PrepareReadFileByPath(&machine, filename))
    {
        // after running the previous function, base_B holds offset of file from file data,
        // and machine.linkedHandle should point to a NitroHandle that is actually
        // an initial segment of a NarcHandle, so the cast is valid
        addr = (const unsigned char*)((NarcHandle*)machine.linkedHandle)->pFileDataStart + machine.fileInfo.startOffset;
        NitroVM_FinishRead(&machine);
    }
    return addr;
}

const void* NarcHandle::GetFileByIndex(unsigned int idx) const
{
    const void* ret = NULL;

    if (idx < pFATBSection->numFiles)
        ret = (const unsigned char*)pFileDataStart + pFATBSection->entries[idx].startOffset;

    return ret;
}

bool PrepareReadFileInNARCByID(NitroVM* vm, NarcHandle* handle, unsigned int id)
{
    bool success = false;

    if (id < handle->pFATBSection->numFiles)
    {
        NitroFileAccessor accessor;
        accessor.handle = &handle->initial;
        accessor.fileID = id;
        success = NitroVM_PrepareReadFileByID(vm, accessor);
    }

    return success;
}