#include "Resource/Script.h"
#include "std_library_functions.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_020ef918 data_020ef808
#endif

extern char data_020ef918[]; // holds a blank string

inline float BitCastIntToFloat(const uint32_t* num)
{
    // should replace with bit_cast if compiling with newer compiler
    return *reinterpret_cast<const float*>(num);
}

bool Script::Initialize()
{
    numSupportedOpcodes_ = 0;
    unsupportedOpcodeProc_ = NULL;
    pCode_ = NULL;
    codeLength_ = 0;
    pHeader_ = NULL;
    maybeInstructionStart_ = NULL;
    dataSection_ = NULL;
    altDataSection_ = 0;
    nextInstruction_ = 0;
    currentInstructionIndex_ = 0;
    unknown_42c_ = false;
    return true;
}

bool Script::SetOpcodeLookup(OpcodeLookupEntry *lookup)
{
    bool success;

    if (lookup == NULL)
        success = false;
    else
    {
        opcodeLookup_ = lookup;

        // We're about to do bubble sort on the instructions, but first
        // count how many there are
        int count = 0;
        OpcodeLookupEntry* countPtr = lookup;
        while (countPtr->opcode != 0)
        {
            countPtr++;
            count++;
        }
        numSupportedOpcodes_ = count;

        int endIndex = count - 1;
        while (true)
        {
            bool changesMade = false;
            OpcodeLookupEntry* swapPtr = lookup;
            for (int idx = 0; idx < endIndex; idx++, swapPtr++)
            {
                if ((unsigned int)swapPtr[0].opcode > swapPtr[1].opcode)
                {
                    changesMade = true;
                    OpcodeLookupEntry temp;
                    memcpy(&temp, &swapPtr[0], sizeof(OpcodeLookupEntry));
                    memcpy(&swapPtr[0], &swapPtr[1], sizeof(OpcodeLookupEntry));
                    memcpy(&swapPtr[1], &temp, sizeof(OpcodeLookupEntry));
                }
            }
            if (changesMade)
                endIndex--;
            else
                break;
        }

        success = true;
    }

    return success;
}

bool Script::Load(const void *code, unsigned int length)
{
    pCode_ = code;
    codeLength_ = length;
    pHeader_ = (const FileHeader*)pCode_;
    maybeInstructionStart_ = (const InstructionPrefix*)((intptr_t)pCode_ + 0x10);
    dataSection_ = (uint8_t*)pCode_ + pHeader_->dataOffset;
    altDataSection_ = 0;
    return true;
}

bool Script::Execute()
{
    if (numSupportedOpcodes_ <= 0 || pCode_ == NULL || codeLength_ <= 0)
        return false;
    {
        currentInstructionIndex_ = 0;
        while (ExecuteSingleInstruction(true) != NULL) {}
        return true;
    }
    return false;
}

const Script::InstructionPrefix* Script::ExecuteSingleInstruction(bool arg)
{
    if (pCode_ == NULL || codeLength_ <= 0 || pHeader_ == NULL)
        return NULL;

    if (currentInstructionIndex_ >= pHeader_->numInstructions)
        return NULL;

    if (nextInstruction_ == NULL)
        nextInstruction_ = maybeInstructionStart_;
    else
    {
        const InstructionPrefix* instruction = nextInstruction_;
        int extraBytes = GetInstructionParamsOffset(instruction) + 4 * instruction->numParams;
        int padding = (4 - (extraBytes % 4)) % 4;
        nextInstruction_ = (const InstructionPrefix*)(
            (intptr_t)instruction + (extraBytes + padding)
        );
    }

    if (nextInstruction_ != NULL && arg)
    {
        int opcode = nextInstruction_->opcode;
        int numArgs = nextInstruction_->numParams;
        const OpcodeLookupEntry* entry = LookupOpcode(opcode);
        bool valid = false;
        if (entry != NULL)
            valid = true;
        else if (!unknown_42c_ && unsupportedOpcodeProc_ != NULL)
            valid = true;

        if (valid)
        {
            const InstructionPrefix* instruction = nextInstruction_;
            const uint8_t* pTypeData = (const uint8_t*)instruction + 3;
            const uint32_t* pSourceArgs = (const uint32_t*)(
                (intptr_t)instruction + GetInstructionParamsOffset(instruction)
            );
            Parameter* pArgsDest = &currentInstructionParams_[0];
            unsigned char currentTypeByte = 0;
            for (int i = 0; i < numArgs; i++)
            {
                if (i >= 128)
                    break;
                if ((i % 4) == 0)
                {
                    if (i > 0)
                        pTypeData++;
                    currentTypeByte = *pTypeData;
                }
                else
                    currentTypeByte >>= 2;

                int currentArgType = currentTypeByte & 3;
                switch (currentArgType)
                {
                case 0: // string
                    pArgsDest->type = 0;
                    pArgsDest->value.str = GetDataString(*pSourceArgs);
                    break;
                case 1: // int
                    pArgsDest->type = 1;
                    pArgsDest->value.i = *pSourceArgs;
                    break;
                case 2: // float
                    pArgsDest->type = 2;
                    pArgsDest->value.f = BitCastIntToFloat(pSourceArgs);
                    break;
                default:
                    continue;
                }
                pSourceArgs++;
                pArgsDest++;
            }
            if (entry != NULL)
                entry->proc(currentInstructionParams_, numArgs);
            else
                unsupportedOpcodeProc_(opcode, currentInstructionParams_, numArgs);
        }
    }
    currentInstructionIndex_++;
    return nextInstruction_;
}

int Script::GetInstructionParamsOffset(const InstructionPrefix* prefix)
{
    // this is super overengineered because numbers are signed and they
    // have no reason to be
    int numParamTypeBits = 2 * prefix->numParams;
    int numParamTypeBytes = (numParamTypeBits / 8) + ((numParamTypeBits % 8) > 0 ? 1 : 0);
    int totalBytes = numParamTypeBytes + 3;
    int padding = (4 - (totalBytes % 4)) % 4;
    return totalBytes + padding;
}

const Script::OpcodeLookupEntry* Script::LookupOpcode(int opcode)
{
    if (numSupportedOpcodes_ <= 0)
        return NULL;

    int searchMin = 0;
    int searchMax = numSupportedOpcodes_ - 1;
    while (searchMin <= searchMax)
    {
        int midpoint = searchMin + ((searchMax - searchMin + 1) / 2);
        const OpcodeLookupEntry* candidate = &opcodeLookup_[midpoint];
        if (opcode == candidate->opcode)
            return candidate;
        else if (opcode < candidate->opcode)
            searchMax = midpoint - 1;
        else
            searchMin = midpoint + 1;
    }

    return NULL;
}

const char* Script::GetDataString(int32_t offset)
{
    if (pCode_ == NULL || codeLength_ <= 0 || pHeader_ == NULL || dataSection_ == NULL)
        return NULL;

    if (offset == -1)
        return data_020ef918;

    if (offset == -2 || offset < 0)
        return NULL;

    if (offset >= pHeader_->dataLength)
        return NULL;

    if (altDataSection_ != 0)
        return (const char*)altDataSection_ + offset;
    else
        return (const char*)dataSection_ + offset;
}

int Script::Parameter::ToInt() const
{
    switch (type)
    {
    case 1:
        return value.i;
    case 2:
        return (int)value.f;
    default:
        return 0;
    }
}

float Script::Parameter::ToFloat() const
{
    switch (type)
    {
    case 1:
        return (float)value.i;
    case 2:
        return value.f;
    }
    return 0.0f;
}

const char* Script::Parameter::ToString() const
{
    switch (type)
    {
    case 0:
        return value.str;
    }
    return NULL;
}

Script::Parameter* Script::Parameter::ToVec3fix(Vector3fix* out)
{
    Script::Parameter* params = this;
    out->x = 4096 * params[0].ToFloat();
    out->y = 4096 * params[1].ToFloat();
    out->z = 4096 * params[2].ToFloat();
    return &params[3];
}

Script::Parameter* Script::Parameter::ToVec3fix16(Vector3fix16 *out)
{
    Script::Parameter* params = this;
    out->x = 4096 * params[0].ToFloat();
    out->y = 4096 * params[1].ToFloat();
    out->z = 4096 * params[2].ToFloat();
    return &params[3];
}