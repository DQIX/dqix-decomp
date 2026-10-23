#include "Resource/PAC.h"

struct PACInnerFileHeader
{
    char name[64]; // not sure about length
    int headerLength;
    unsigned int fileLength;
    int offsetToNextFile;

    inline void* GetFile()
    {
        return (char*)this + headerLength;
    }

    inline PACInnerFileHeader* Next()
    {
        return (PACInnerFileHeader*)((char*)this + offsetToNextFile);
    }

    inline CBool IsValid() const { return name[0] != 0; }
};

PACInnerFileHeader* GetPACInnerFileHeader(void* pac, const char* filename)
{
    if (pac == NULL)
        return NULL;
    if (filename == NULL)
        return NULL;

    if (filename[0] == 0)
        return NULL;

    const char* withoutDirectory = filename;
    while (*filename != 0)
    {
        if (*filename == '/')
            withoutDirectory = filename + 1;
        filename++;
    }

    for (PACInnerFileHeader* file = (PACInnerFileHeader*)pac; file->name[0] != 0; file = file->Next())
    {
        if (strcmp(file->name, withoutDirectory) == 0)
            return file;
    }

    return NULL;
}

void* GetFileInPAC(void* pac, const char* filename, unsigned int* outLength)
{
    PACInnerFileHeader* header = GetPACInnerFileHeader(pac, filename);
    if (header == NULL)
        return NULL;

    if (outLength != NULL)
        *outLength = header->fileLength;
    return header->GetFile();
}

void* GetFileInPACByIndex(void* pac, unsigned int idx, const char** outFilenamePtr, unsigned int* outLength)
{
    if (pac == NULL)
        return NULL;
    PACInnerFileHeader* header = (PACInnerFileHeader*)pac;
    unsigned int loopIndex = 0;
    while (header->name[0] != 0)
    {
        if (idx == loopIndex)
        {
            void* file = header->GetFile();
            if (outLength != NULL)
                *outLength = header->fileLength;
            *outFilenamePtr = header->name;
            return file;
        }
        header = header->Next();
        loopIndex++;
    }

    return NULL;
}

int GetFilesInPACByExtension(void* pac, const char* extension, void** outFiles,
    int maxNumFiles, unsigned int* outLengths, const char** outFilenamePtrs)
{
    int numFound = 0;
    for (int i = 0; true; i++)
    {
        unsigned int thisFileLength;
        const char* thisFileName;
        void* thisFileData = GetFileInPACByIndex(pac, i, &thisFileName, &thisFileLength);
        if (thisFileData == NULL)
            break;
        const char* thisExtension = thisFileName;
        while (*thisExtension != 0)
        {
            if (*thisExtension == '.')
            {
                thisExtension++;
                break;
            }
            thisExtension++;
        }

        if (strcmp(extension, thisExtension) != 0)
            continue;

        outFiles[numFound] = thisFileData;
        if (outFilenamePtrs != NULL)
            outFilenamePtrs[numFound] = thisFileName;
        if (outLengths != NULL)
            outLengths[numFound] = thisFileLength;
        numFound++;

        if (numFound >= maxNumFiles)
            break;
    }
    return numFound;
}

int GetNumFilesInPAC(const void* pac)
{
    int count = 0;
    for (PACInnerFileHeader* header = (PACInnerFileHeader*)pac; header->IsValid(); header = header->Next())
        count++;
    return count;
}