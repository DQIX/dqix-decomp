#include "Graphics/NSBXX/RenderCommands_Common.h"

static inline int TestBitfieldBit(const unsigned int* bitfield, unsigned int idx)
{
    return bitfield[idx >> 5] & (1 << (idx & 31));
}

static inline int SetBitfieldBit(unsigned int* bitfield, unsigned int idx)
{
    bitfield[idx >> 5] |= (1 << (idx & 31));
}

// not a match, 64-bit arithmetic is ruining everything.
// notably there is no hook/callback stuff in this command.
void RenderCommand_9(RenderCommandHandler* handler, int modifier)
{
    int64_t weight = 0;
    NSBXXInternalModel* model = handler->modelContext_->internalModel_;    
    const NSBXXInvBindMatrix* invBindMatrices = (NSBXXInvBindMatrix*)((intptr_t)model + model->inverseBindsOffset_);
    unsigned int numTerms = handler->instructionPointer_[2];
    NSBXXInvBindMatrix localCopy;
    
    uint8_t* instructionPtr = handler->instructionPointer_ + 3;
    
    func_020ca458(0, &localCopy, sizeof(NSBXXInvBindMatrix));
    SendQueuedDataToGeometryFifo();

    GXFIFO_MATRIX_MODE = 0; // projection
    GXFIFO_MATRIX_STORE = 1;
    GXFIFO_MATRIX_IDENTITY = 0;
    GXFIFO_MATRIX_MODE = 2; // position+vector

    Struct_0210b678* ptr_r7;
    fix32_t* mat3x3_unaffr8;
    
    unsigned int termCounter = 0;
    for (termCounter = 0; termCounter < numTerms; termCounter++)
    {
        unsigned int inverseBindIndex = instructionPtr[1];
        unsigned int offset = inverseBindIndex * sizeof(Struct_0210b678);
        
        int inBitfield = TestBitfieldBit(handler->invBindBitfield_, inverseBindIndex);
        ptr_r7 = (Struct_0210b678*)((char*)data_0210b678 + offset);
        if (!inBitfield)
        {
            SetBitfieldBit(handler->invBindBitfield_, inverseBindIndex);
            // retrieve the local-to-world matrix from the stack
            GXFIFO_MATRIX_GET = instructionPtr[0];
            GXFIFO_MATRIX_MODE = 1; // position
            func_020c51a4(invBindMatrices[inverseBindIndex].mat4x3.entries);
        }

        if (termCounter != 0)
        {
            localCopy.mat3x3.entries[0] += (weight * mat3x3_unaffr8[0]) >> 12;
            localCopy.mat3x3.entries[1] += (weight * mat3x3_unaffr8[1]) >> 12;
            localCopy.mat3x3.entries[2] += (weight * mat3x3_unaffr8[2]) >> 12;
            localCopy.mat3x3.entries[3] += (weight * mat3x3_unaffr8[3]) >> 12;
            localCopy.mat3x3.entries[4] += (weight * mat3x3_unaffr8[4]) >> 12;
            localCopy.mat3x3.entries[5] += (weight * mat3x3_unaffr8[5]) >> 12;
            localCopy.mat3x3.entries[6] += (weight * mat3x3_unaffr8[6]) >> 12;
            localCopy.mat3x3.entries[7] += (weight * mat3x3_unaffr8[7]) >> 12;
            localCopy.mat3x3.entries[8] += (weight * mat3x3_unaffr8[8]) >> 12;
        }
        if (!inBitfield)
        {
            while (func_020c54fc(&ptr_r7->mat4x4) != 0) {}
            GXFIFO_MATRIX_MODE = 2; // position+vector
            func_020c51c0(invBindMatrices[inverseBindIndex].mat3x3.entries);
        }
        weight = instructionPtr[2] << 4; // as int -> fixed point this is division by 256
        localCopy.mat4x3.entries[0] +=  (weight * ptr_r7->mat4x4.entries[ 0]) >> 12;
        localCopy.mat4x3.entries[1] +=  (weight * ptr_r7->mat4x4.entries[ 1]) >> 12;
        localCopy.mat4x3.entries[2] +=  (weight * ptr_r7->mat4x4.entries[ 2]) >> 12;
        localCopy.mat4x3.entries[3] +=  (weight * ptr_r7->mat4x4.entries[ 4]) >> 12;
        localCopy.mat4x3.entries[4] +=  (weight * ptr_r7->mat4x4.entries[ 5]) >> 12;
        localCopy.mat4x3.entries[5] +=  (weight * ptr_r7->mat4x4.entries[ 6]) >> 12;
        localCopy.mat4x3.entries[6] +=  (weight * ptr_r7->mat4x4.entries[ 8]) >> 12;
        localCopy.mat4x3.entries[7] +=  (weight * ptr_r7->mat4x4.entries[ 9]) >> 12;
        localCopy.mat4x3.entries[8] +=  (weight * ptr_r7->mat4x4.entries[10]) >> 12;
        localCopy.mat4x3.entries[9] +=  (weight * ptr_r7->mat4x4.entries[12]) >> 12;
        localCopy.mat4x3.entries[10] += (weight * ptr_r7->mat4x4.entries[13]) >> 12;
        localCopy.mat4x3.entries[11] += (weight * ptr_r7->mat4x4.entries[14]) >> 12;

        instructionPtr += 3;
        // very stupid but this was the only way to avoid optimizing out
        // the data_0210b678+0x40 reference
        mat3x3_unaffr8 = (fix32_t*)((char*)&data_0210b678[0].mat3x3.entries[0] + offset);
        if (!inBitfield)
        {
            while (func_020c552c(mat3x3_unaffr8) != 0) {}
        }
    }

    localCopy.mat3x3.entries[0] += (weight * mat3x3_unaffr8[0]) >> 12;
    localCopy.mat3x3.entries[1] += (weight * mat3x3_unaffr8[1]) >> 12;
    localCopy.mat3x3.entries[2] += (weight * mat3x3_unaffr8[2]) >> 12;
    localCopy.mat3x3.entries[3] += (weight * mat3x3_unaffr8[3]) >> 12;
    localCopy.mat3x3.entries[4] += (weight * mat3x3_unaffr8[4]) >> 12;
    localCopy.mat3x3.entries[5] += (weight * mat3x3_unaffr8[5]) >> 12;
    localCopy.mat3x3.entries[6] += (weight * mat3x3_unaffr8[6]) >> 12;
    localCopy.mat3x3.entries[7] += (weight * mat3x3_unaffr8[7]) >> 12;
    localCopy.mat3x3.entries[8] += (weight * mat3x3_unaffr8[8]) >> 12;

    func_020c5188(localCopy.mat3x3.entries); // isn't this supposed to take a 3x4 matrix?
    GXFIFO_MATRIX_MODE = 1;
    func_020c5188(localCopy.mat4x3.entries);
    GXFIFO_MATRIX_MODE = 0;
    GXFIFO_MATRIX_GET = 1;
    GXFIFO_MATRIX_MODE = 2;
    GXFIFO_MATRIX_STORE = handler->instructionPointer_[1];
    handler->instructionPointer_ += 3 + handler->instructionPointer_[2] * 3;
}