#pragma once

#include "std_library_functions.h"
#include "../Graphics/Vector.h"

// General purpose object for running various script-like files.
// It seems the types of commands can be specified by the user.
class Script
{
public:
    struct Parameter
    {
        int type; // 0 = string, 1 = int, 2 = float?
        union
        {
            const char* str;
            int i;
            float f;
        } value;

        int ToInt() const;
        float ToFloat() const;
        const char* ToString() const;
        Parameter* ToVec3fix(Vector3fix* out);
        Parameter* ToVec3fix16(Vector3fix16* out);
    };

    typedef int (*OpcodeProc)(Parameter*, int);

    struct OpcodeLookupEntry
    {
        int opcode;
        OpcodeProc proc;
    };

    // Within the script file, each instruction consists of this prefix,
    // then some data specifying the type of each parameter. This data uses 2
    // bits per entry. Then pad to a 4-byte boundary, then you get 4 bytes
    // per parameter (32-bit int or float for int/float, and an offset within
    // the data section for a string).
    struct InstructionPrefix
    {
        uint16_t opcode;
        uint8_t numParams;
    };

    struct FileHeader
    {
        int32_t numInstructions;
        uint32_t dataOffset;
        int32_t dataLength;
        uint32_t maybeNumStrings;
    };

    int numSupportedOpcodes_;
    OpcodeLookupEntry* opcodeLookup_; // sorted array
    int (*unsupportedOpcodeProc_)(unsigned short opcode, Parameter* params, int numParams);
    const void* pCode_;
    int codeLength_;
    const FileHeader* pHeader_;
    const InstructionPrefix* maybeInstructionStart_;
    const void* dataSection_;
    const void* altDataSection_;
    const InstructionPrefix* nextInstruction_;
    int currentInstructionIndex_;
    Parameter currentInstructionParams_[128];
    bool unknown_42c_; // might be unsigned char

    bool Initialize();
    bool SetOpcodeLookup(OpcodeLookupEntry* lookup);
    bool Load(const void* code, unsigned int length);
    bool Execute();

private:
    const InstructionPrefix* ExecuteSingleInstruction(bool arg);
    int GetInstructionParamsOffset(const InstructionPrefix* prefix);
    const OpcodeLookupEntry* LookupOpcode(int opcode);
    const char* GetDataString(int32_t offset);
};