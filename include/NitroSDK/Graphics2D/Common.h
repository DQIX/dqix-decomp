#pragma once

#include <std_library_functions.h>

struct Nitro2DInnerFileHeader;

struct Nitro2DFileHeader
{
    uint32_t signature;
    uint16_t byteOrderMark;
    uint16_t version;
    uint32_t fileSize;
    uint16_t firstSubfileOffset;
    uint16_t numSubfiles;

    Nitro2DInnerFileHeader* GetFirstSubfile()
    {
        return (Nitro2DInnerFileHeader*)((char*)this + firstSubfileOffset);
    }
};

struct Nitro2DInnerFileHeader
{
    uint32_t signature;
    uint32_t fileSize;

    Nitro2DInnerFileHeader* Next()
    {
        return (Nitro2DInnerFileHeader*)((char*)this + fileSize);
    }

    void* GetInterior()
    {
        return this + 1;
    }
};