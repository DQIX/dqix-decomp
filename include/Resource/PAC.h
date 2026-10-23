#pragma once

#include <std_library_functions.h>
#include <globaldefs.h>

void* GetFileInPAC(void* pac, const char* filename, unsigned int* outLength);

inline const void* GetFileInPAC(const void* pac, const char* filename, unsigned int* outLength)
{
    return GetFileInPAC(const_cast<void*>(pac), filename, outLength);
}

void* GetFileInPACByIndex(void* pac, unsigned int idx, const char** outFilenamePtr, unsigned int* outLength);

inline const void* GetFileInPACByIndex(const void* pac, const unsigned int idx, const char** outFilenamePtr, unsigned int* outLength)
{
    return GetFileInPACByIndex(const_cast<void*>(pac), idx, outFilenamePtr, outLength);
}

int GetFilesInPACByExtension(void* pac, const char* extension, void** outFiles,
    int maxNumFiles, unsigned int* outLengths, const char** outFilenamePtrs);

inline int GetFilesInPACByExtension(const void* pac, const char* extension, const void** outFiles,
    int maxNumFiles, unsigned int* outLengths, const char** outFilenamePtrs)
{
    // to do: fix UB from type punning
    return GetFilesInPACByExtension(const_cast<void*>(pac), extension,
        const_cast<void**>(outFiles), maxNumFiles, outLengths, outFilenamePtrs);
}

int GetNumFilesInPAC(const void* pac);