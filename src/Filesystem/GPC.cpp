#include "Filesystem/GPC.h"
#include <globaldefs.h>

#pragma dont_inline on

#if defined(jpn)
#define func_02075090 func_0207621c
#define func_020d84f8 func_020d9e5c
#define func_020d8524 func_020d9e88

#define data_020f27c0 data_020f297c
#define data_020f27e8 data_020f29a4
#define data_020f27f0 data_020f29ac
#define data_020f27f8 data_020f29b4
#endif

extern "C"
{
    // Calculate CRC for a null-terminated string
    unsigned int func_01ff860c(const char*);

    // always zero. Based on where it is, and the fact that the argument
    // is a file path, maybe it used to check if the file was loaded into 
    // memory? But now it just says no
    int func_02075090(const char*);

    // zero memory and flush
    void func_020d84f8(void*, unsigned int);
    // memcpy and flush
    void func_020d8524(void*, const void*, unsigned int);
}

struct GPCImplementationData
{
    const char* pRevisionNumberString; // "17659 $"
    const char* pRevisionString; // "$Revision: 17659 $"
    unsigned int gpc0Signature;
    unsigned int gpc1Signature;
    unsigned int gpc2Signature;
} extern data_020f27c0;

// GPC0 signature
extern char data_020f27e8[8];
// GPC1 signature
extern char data_020f27f0[8];
// GPC2 signature
extern char data_020f27f8[8];

extern "C" bool GetUnknownGP2Data_020d8fcc(void** pOutPtr, unsigned int* pOutNumber, GPCFile* gpc, const GPCFile::FileEntry* file)
{
    if (file == NULL)
        return false;
    unsigned int offset = gpc->header.firstFileOffset * 4 + file->offset * 4;
    *pOutPtr = (unsigned char*)gpc + offset;
    *pOutNumber = file->compressedSizeBytes;
    return true;
}

void* GPCFile::GetMainDataStart()
{
    unsigned int* ptr = (unsigned int*)this;
    return &ptr[header.headerLength];
}

GPCFile::FileEntry* GPCFile::GetFileByCRC(unsigned int crc)
{
    if (header.headerLength == 0)
        return NULL;
    if (crc == 0)
        return NULL;

    FileEntry* entryList = (FileEntry*)GetMainDataStart();
    
    unsigned int found = -1;
    unsigned int currentIdx = 0;
    
    int rightAddition = 1 << header.binarySearchStepCount;
    while (rightAddition > 0)
    {
        rightAddition >>= 1;
        unsigned int candidateRightIndex = currentIdx + rightAddition;
        if (candidateRightIndex < header.fileCount)
        {
            int result;
            if (crc == entryList[candidateRightIndex].crc)
                result = 0;
            else if (crc < entryList[candidateRightIndex].crc)
                result = -1;
            else
                result = 1;

            if (result == 0)
                found = candidateRightIndex;
            else if (result > 0)
                currentIdx += rightAddition;
        }
    }

    return &entryList[found];
}

extern "C" bool GetUnknownGP2InnerFileDataByName(void** pOutPtr,
    unsigned int* pOutNumber, GPCFile* gpc, const char* name)
{
    unsigned int crc = func_01ff860c(name);
    return GetUnknownGP2InnerFileDataByCRC(pOutPtr, pOutNumber, gpc, crc);
}

extern "C" bool GetUnknownGP2InnerFileDataByCRC(void** pOutPtr,
    unsigned int* pOutNumber, GPCFile* gpc, unsigned int crc)
{
    *pOutPtr = NULL;
    *pOutNumber = 0;
    if (gpc == NULL || gpc->header.signature != data_020f27c0.gpc1Signature)
        return false;

    GPCFile::FileEntry* entry = gpc->GetFileByCRC(crc);
    return GetUnknownGP2Data_020d8fcc(pOutPtr, pOutNumber, gpc, entry);
}

void ZeroInitGPCPointer(GPCFile** ppGPC)
{
    *ppGPC = NULL;
}

void ZeroDestroyGPCPointer(GPCFile** ppGPC)
{
    *ppGPC = NULL;
}

bool CheckGPCPairValid_020d917c(GPCFile* const& gpc, ExtendedNitroVM& machine)
{
    if (gpc != NULL && machine.IsInReadableState())
        return true;
    return false;
}

bool ResetGPCPair(GPCFile **ppGPC, ExtendedNitroVM &machine)
{
    if (machine.IsInReadableState())
        machine.Close();
    *ppGPC = NULL;
    return true;
}

bool LoadAndDecompressGPCHeaderAndInnerFileInfo(GPCFile **ppGPC, ExtendedNitroVM &machine,
    const char *gpcFilePath, void *outputBuffer, unsigned int &outDecompressedLength,
    unsigned int outputCapacity, bool rightAlignOutput, bool *pOutReadSuccessful)
{
    

    if (pOutReadSuccessful != NULL)
        *pOutReadSuccessful = false;

    bool success = false;
    outDecompressedLength = 0;

    ResetGPCPair(ppGPC, machine);
    int unknownPurposeButAlwaysZero = func_02075090(gpcFilePath);
    {
    if (!machine.Open(gpcFilePath, unknownPurposeButAlwaysZero != 0))
        goto end;

    if (pOutReadSuccessful != NULL)
        *pOutReadSuccessful = true;

    GPCFile::Header header;
    func_020d84f8(&header, sizeof(GPCFile::Header));
    unsigned int desiredSignature = data_020f27c0.gpc2Signature;
    header.signature = desiredSignature;
    if (!machine.Read(&header, sizeof(GPCFile::Header)))
        goto end;

    if (machine.isBadState != 0 || header.signature != desiredSignature)
        goto end;

    unsigned int totalLength = (header.headerLength + header.decompressedFileInfoLength) * 4;
    unsigned int longerLength = totalLength + 4;
    if (longerLength > outputCapacity)
        goto end;

    if (rightAlignOutput)
        outputBuffer = (void*)((((int)outputBuffer + outputCapacity) & ~3) - longerLength);
    func_020d8524(outputBuffer, &header, sizeof(GPCFile::Header));
    ((GPCFile::Header*)outputBuffer)->headerLength = 0;
    ((GPCFile::Header*)outputBuffer)->fileInfoLength = 0;
    unsigned int decompressProcOutputLength = 0;

    if (!machine.DecompressBytes((unsigned char*)outputBuffer + sizeof(GPCFile::Header),
        decompressProcOutputLength,
        header.fileInfoLength * 4 - sizeof(GPCFile::Header),
        longerLength - sizeof(GPCFile::Header)))
        goto end;
    
    GPCFile::Header* outputBufferAsHeader = (GPCFile::Header*)outputBuffer; 
    // goofy way to write sizeof(GPCFile::Header) / 4 == 5
    outputBufferAsHeader->headerLength = ((unsigned int)(outputBufferAsHeader + 1) - (unsigned int)outputBufferAsHeader) / 4;
    *ppGPC = (GPCFile*)outputBuffer;
    outDecompressedLength = longerLength;
    success = true;
    }
end:
    if (!success)
        ResetGPCPair(ppGPC, machine);
    return success;
}

unsigned int GetGPCInnerFileLength(ExtendedNitroVM& machine,
    const GPCFile::Header& header, const GPCFile::FileEntry* file)
{
    unsigned int length;

    if (file == NULL)
        length = 0;
    else
    {
        machine.Seek((header.firstFileOffset + file->offset) * 4);
        unsigned int prefixU32 = 0;
        machine.Read(&prefixU32, 4);
        if (machine.isBadState == 0 && header.isUncompressed)
            length = file->compressedSizeBytes;
        else
            length = ((CompressionPrefix*)&prefixU32)->GetDecompressedLength();
    }
    
    return length;
}

unsigned int GetGPCInnerFileLengthByName(GPCFile* const& gpc, 
    ExtendedNitroVM& machine, const char* filename)
{
    unsigned int crc = func_01ff860c(filename);
    return GetGPCInnerFileLengthByCRC(gpc, machine, crc);
}

unsigned int GetGPCInnerFileLengthByCRC(GPCFile *const &gpc, 
    ExtendedNitroVM &machine, unsigned int crc)
{
    if (gpc == NULL || !machine.IsInReadableState())
        return 0;
    const GPCFile::Header& header = gpc->header;
    GPCFile::FileEntry* entry = gpc->GetFileByCRC(crc);
    return GetGPCInnerFileLength(machine, header, entry);
}

bool DecompressFileFromGPC(ExtendedNitroVM &machine, void *outputBuffer,
    unsigned int &outDecompressedLength, unsigned int outputCapacity,
    const GPCFile::Header &gpcHeader, const GPCFile::FileEntry *file)
{
    if (file == NULL)
        return false;

    machine.Seek((gpcHeader.firstFileOffset + file->offset) * 4);
    if (gpcHeader.isUncompressed)
    {
        // note: compressedSizeBytes is actually the uncompressed size
        // (because we aren't compressed on this branch!)
        bool success;
        if (outputCapacity >= file->compressedSizeBytes && 
            machine.Read(outputBuffer, file->compressedSizeBytes) &&
            machine.isBadState == 0)
        {
            success = true;
            outDecompressedLength = file->compressedSizeBytes;
        }
        else
            success = false;
        
        return success;
    }
    else
    {
        return machine.DecompressBytes(outputBuffer, outDecompressedLength, 
            file->compressedSizeBytes, outputCapacity);
    }
}

bool DecompressFileFromGPCByName(GPCFile* const& gpc, ExtendedNitroVM& machine,
    void* outputBuffer, unsigned int& outDecompressedLength,
    unsigned int outputCapacity, const char* filename)
{
    unsigned int crc = func_01ff860c(filename);
    return DecompressFileFromGPCByCRC(gpc, machine, outputBuffer,
        outDecompressedLength, outputCapacity, crc);
}

bool DecompressFileFromGPCByCRC(GPCFile* const& gpc, ExtendedNitroVM& machine,
    void* outputBuffer, unsigned int& outDecompressedLength,
    unsigned int outputCapacity, unsigned int crc)
{
    outDecompressedLength = 0;
    if (gpc == NULL || !machine.IsInReadableState())
        return false;
    
    const GPCFile::Header& header = gpc->header;
    GPCFile::FileEntry* entry = gpc->GetFileByCRC(crc);
    return DecompressFileFromGPC(machine, outputBuffer,
        outDecompressedLength, outputCapacity, header, entry);
}

void GPCReadPair::ZeroInitialize()
{
    pGPCFile = NULL;
    machine.ZeroInitialize();
}

void GPCReadPair::ZeroInitializeMachine()
{
    machine.ZeroInitialize();
}

bool GPCReadPair::Reset()
{
    return ResetGPCPair(&pGPCFile, machine);
}

void CopyGPCSignature(unsigned int *dst, unsigned int *src)
{
    *dst = *src;
}

unsigned int* SetGPCSignatureGPC0(unsigned int* dst)
{
    CopyGPCSignatureFromString(dst, data_020f27e8);
    return dst;
}

void CopyGPCSignatureFromString(unsigned int* dst, const char* src)
{
    char* dstString = reinterpret_cast<char*>(dst);
    dstString[0] = src[0];
    dstString[1] = src[1];
    dstString[2] = src[2];
    dstString[3] = src[3];
}

unsigned int* SetGPCSignatureGPC1(unsigned int* dst)
{
    CopyGPCSignatureFromString(dst, data_020f27f0);
    return dst;
}

unsigned int* SetGPCSignatureGPC2(unsigned int* dst)
{
    CopyGPCSignatureFromString(dst, data_020f27f8);
    return dst;
}