#pragma once

#include "ExtendedNitroVM.h"

class GPCFile
{
public:
    // lengths are in words (uint32s), not bytes
    struct Header
    {
        unsigned int signature;
        unsigned short fileCount : 12;
        unsigned short binarySearchStepCount : 4;
        unsigned short headerLength;
        unsigned short fileInfoLength;
        unsigned short firstFileOffset;
        unsigned short decompressedFileInfoLength;
        unsigned short decompressedFilenameLength;
        unsigned int totalFileSize : 28;
        unsigned int isUncompressed : 1; // 0 = compressed, 1 = uncompressed
    } header;

    struct FileEntry
    {
        unsigned int crc;
        unsigned int offset : 24;
        unsigned int unknown_7 : 8;
        unsigned int compressedSizeBytes : 24;
        unsigned int unknown_B : 8;
    };

    void* GetMainDataStart();

    // I think this requires the data to be decompressed already
    FileEntry* GetFileByCRC(unsigned int crc);
};

extern "C" bool GetUnknownGP2InnerFileDataByName(void** pOutPtr, 
    unsigned int* pOutNumber, GPCFile* gpc, const char* name);
extern "C" bool GetUnknownGP2InnerFileDataByCRC(void** pOutPtr, 
    unsigned int* pOutNumber, GPCFile* gpc, unsigned int crc);

// These two functions are identical (they write 0 into the pointer.)
// But the uses of it heavily indicate initialize/destroy respectively
void ZeroInitGPCPointer(GPCFile** ppGPC);
void ZeroDestroyGPCPointer(GPCFile** ppGPC);

extern "C" bool CheckGPCPairValid_020d917c(GPCFile* const& gpc, ExtendedNitroVM& machine);
bool ResetGPCPair(GPCFile** ppGPC, ExtendedNitroVM& machine);

// decompresses the header (0x14 bytes) and the FileEntry array (0x0C bytes
// times number of inner files) given the path of a filesystem file.
// Returns true if both loading the file and performing the decompression
// succeeded. Argument #8 outputs true if loading the file succeeded
// (regardless of decompression result).
bool LoadAndDecompressGPCHeaderAndInnerFileInfo(GPCFile** ppGPC, ExtendedNitroVM& machine,
    const char* gpcFilePath, void* outputBuffer, unsigned int& outDecompressedLength,
    unsigned int outputCapacity, bool rightAlignOutput, bool* outReadSuccessful);

unsigned int GetGPCInnerFileLength(ExtendedNitroVM& machine, 
    const GPCFile::Header& header, const GPCFile::FileEntry* file);

unsigned int GetGPCInnerFileLengthByName(GPCFile* const& gpc, ExtendedNitroVM& machine, const char* filename);
unsigned int GetGPCInnerFileLengthByCRC(GPCFile* const& gpc, ExtendedNitroVM& machine, unsigned int crc);

bool DecompressFileFromGPC(ExtendedNitroVM& machine, void* outputBuffer, 
    unsigned int& outDecompressedLength, unsigned int outputCapacity,
    const GPCFile::Header& gpcHeader, const GPCFile::FileEntry* file);

bool DecompressFileFromGPCByName(GPCFile* const& gpc, ExtendedNitroVM& machine,
    void* outputBuffer, unsigned int& outDecompressedLength,
    unsigned int outputCapacity, const char* filename);
bool DecompressFileFromGPCByCRC(GPCFile* const& gpc, ExtendedNitroVM& machine,
    void* outputBuffer, unsigned int& outDecompressedLength,
    unsigned int outputCapacity, unsigned int crc);

// This exists as its own type, but many functions that we'd expect to
// act on it instead take a pair of references/pointers to each member
class GPCReadPair
{
public:
    GPCFile* pGPCFile;
    ExtendedNitroVM machine;

    void ZeroInitialize();
    void ZeroInitializeMachine();
    bool Reset();
};

void CopyGPCSignature(unsigned int* dst, const unsigned int* src);
unsigned int* SetGPCSignatureGPC0(unsigned int* dst);
void CopyGPCSignatureFromString(unsigned int* dst, const char* src);
unsigned int* SetGPCSignatureGPC1(unsigned int* dst);
unsigned int* SetGPCSignatureGPC2(unsigned int* dst);
