#pragma once

struct GPCSignature
{
    unsigned int value;

    GPCSignature() {}
    GPCSignature(const char* str)
    {
        char* dst = reinterpret_cast<char*>(&value);
        dst[0] = str[0];
        dst[1] = str[1];
        dst[2] = str[2];
        dst[3] = str[3];
    }
};

template<int n>
struct VersionedGPCSignature : GPCSignature
{
    static char bytes[8];
    VersionedGPCSignature() : GPCSignature(bytes) {}
};

extern char g_gpcVersion[];