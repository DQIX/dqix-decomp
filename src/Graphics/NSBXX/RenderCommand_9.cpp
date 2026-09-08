#include "Graphics/NSBXX/RenderCommands_Common.h"

// not a match, 64-bit arithmetic is ruining everything.
// notably there is no hook/callback stuff in this command.
extern "C" void RenderCommand_9(RenderCommandHandler* handler, int modifier)
{
    NSBXXInternalModel* model = handler->modelContext_->internalModel_;
    int numTerms = handler->instructionPointer_[2];
    fix32_t weight = 0;
    NSBXXInvBindMatrix* invBindMatrices = (NSBXXInvBindMatrix*)((intptr_t)model + model->inverseBindsOffset_);
    NSBXXInvBindMatrix localCopy;
    
    uint8_t* instructionPtr = handler->instructionPointer_ + 3;
    
    func_020ca458(0, &localCopy, sizeof(NSBXXInvBindMatrix));
    SendQueuedDataToGeometryFifo();

    GXFIFO_MATRIX_MODE = 0; // projection
    GXFIFO_MATRIX_STORE = 1;
    GXFIFO_MATRIX_IDENTITY = 0;
    GXFIFO_MATRIX_MODE = 2; // position+vector

    fix32_t* mat3x3_unaffr8;
    
    unsigned int termCounter = 0;
    if (numTerms > UNSIGNED_ZERO())
    {
        do
        {
            Struct_0210b678* ptr_r7;
            unsigned int offset;
            offset = instructionPtr[1] * sizeof(Struct_0210b678);
            ptr_r7 = (Struct_0210b678*)((intptr_t)data_0210b678 + offset);
            unsigned int inverseBindIndex = instructionPtr[1];
            unsigned int inBitfield = handler->invBindBitfield_[inverseBindIndex >> 5] & (1 << (inverseBindIndex & 0x1f));
            if (!inBitfield)
            {
                handler->invBindBitfield_[inverseBindIndex >> 5] |= (1 << (inverseBindIndex & 0x1f));
                // retrieve the local-to-world matrix from the stack
                GXFIFO_MATRIX_GET = instructionPtr[0];
                GXFIFO_MATRIX_MODE = 1; // position
                func_020c51a4(invBindMatrices[inverseBindIndex].mat4x3.entries);
            }
            if (termCounter != 0)
            {
                unsigned uWeight = weight;
                localCopy.mat3x3.entries[0] += (uWeight * (int64_t)mat3x3_unaffr8[0]) >> 12;
                localCopy.mat3x3.entries[1] += (uWeight * (int64_t)mat3x3_unaffr8[1]) >> 12;
                localCopy.mat3x3.entries[2] += (uWeight * (int64_t)mat3x3_unaffr8[2]) >> 12;
                localCopy.mat3x3.entries[3] += (uWeight * (int64_t)mat3x3_unaffr8[3]) >> 12;
                localCopy.mat3x3.entries[4] += (uWeight * (int64_t)mat3x3_unaffr8[4]) >> 12;
                localCopy.mat3x3.entries[5] += (uWeight * (int64_t)mat3x3_unaffr8[5]) >> 12;
                localCopy.mat3x3.entries[6] += (uWeight * (int64_t)mat3x3_unaffr8[6]) >> 12;
                localCopy.mat3x3.entries[7] += (uWeight * (int64_t)mat3x3_unaffr8[7]) >> 12;
                localCopy.mat3x3.entries[8] += (uWeight * (int64_t)mat3x3_unaffr8[8]) >> 12;
            }
            if (!inBitfield)
            {
                while (func_020c54fc(&ptr_r7->mat4x4) != 0) {}
                GXFIFO_MATRIX_MODE = 2; // position+vector
                func_020c51c0(invBindMatrices[inverseBindIndex].mat3x3.entries);
            }
            weight = instructionPtr[5] << 4; // as int -> fixed point this is division by 256
            localCopy.mat4x3.entries[0] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 0]) >> 12;
            localCopy.mat4x3.entries[1] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 1]) >> 12;
            localCopy.mat4x3.entries[2] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 2]) >> 12;
            localCopy.mat4x3.entries[3] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 4]) >> 12;
            localCopy.mat4x3.entries[4] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 5]) >> 12;
            localCopy.mat4x3.entries[5] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 6]) >> 12;
            localCopy.mat4x3.entries[6] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 8]) >> 12;
            localCopy.mat4x3.entries[7] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[ 9]) >> 12;
            localCopy.mat4x3.entries[8] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[10]) >> 12;
            localCopy.mat4x3.entries[9] +=  (weight * (int64_t)ptr_r7->mat4x4.entries[12]) >> 12;
            localCopy.mat4x3.entries[10] += (weight * (int64_t)ptr_r7->mat4x4.entries[13]) >> 12;
            localCopy.mat4x3.entries[11] += (weight * (int64_t)ptr_r7->mat4x4.entries[14]) >> 12;

            instructionPtr += 3;
            // very stupid but this was the only way to avoid optimizing out
            // the data_0210b678+0x40 reference
            mat3x3_unaffr8 = (fix32_t*)((intptr_t)&data_0210b678[0].mat3x3.entries[0] + offset);
            if (!inBitfield)
            {
                while (func_020c552c(mat3x3_unaffr8) != 0) {}
            }
            termCounter++;
            
        } while (termCounter < numTerms);
    }

    unsigned int uWeight = weight;
    localCopy.mat3x3.entries[0] += (uWeight * (int64_t)mat3x3_unaffr8[0]) >> 12;
    localCopy.mat3x3.entries[1] += (uWeight * (int64_t)mat3x3_unaffr8[1]) >> 12;
    localCopy.mat3x3.entries[2] += (uWeight * (int64_t)mat3x3_unaffr8[2]) >> 12;
    localCopy.mat3x3.entries[3] += (uWeight * (int64_t)mat3x3_unaffr8[3]) >> 12;
    localCopy.mat3x3.entries[4] += (uWeight * (int64_t)mat3x3_unaffr8[4]) >> 12;
    localCopy.mat3x3.entries[5] += (uWeight * (int64_t)mat3x3_unaffr8[5]) >> 12;
    localCopy.mat3x3.entries[6] += (uWeight * (int64_t)mat3x3_unaffr8[6]) >> 12;
    localCopy.mat3x3.entries[7] += (uWeight * (int64_t)mat3x3_unaffr8[7]) >> 12;
    localCopy.mat3x3.entries[8] += (uWeight * (int64_t)mat3x3_unaffr8[8]) >> 12;

    func_020c5188(localCopy.mat3x3.entries); // isn't this supposed to take a 3x4 matrix?
    GXFIFO_MATRIX_MODE = 1;
    func_020c5188(localCopy.mat4x3.entries);
    GXFIFO_MATRIX_MODE = 0;
    GXFIFO_MATRIX_GET = 1;
    GXFIFO_MATRIX_MODE = 2;
    GXFIFO_MATRIX_STORE = handler->instructionPointer_[1];
    handler->instructionPointer_ += (handler->instructionPointer_[2] + 1) * 3;
}