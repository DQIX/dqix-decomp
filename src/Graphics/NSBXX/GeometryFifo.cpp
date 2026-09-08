#include "Graphics/NSBXX/GeometryFifo.h"
#include "System/Graphics.h"
#include "Graphics/NSBXX/RenderCommands.h"
#include "Graphics/NSBXX/RenderConfig.h"
#include "System/DMA.h"
#include <globaldefs.h>

#pragma optimize_for_size off

#if defined(jpn)
#define func_020c2034 func_020c3b00
#define func_020c2208 func_020c3cd4
#define func_020c2c38 func_020c4704
#define func_020c2c94 func_020c4760
#define func_020c51dc func_020c6ca8
#define func_020c54fc func_020c6fc8
#define func_020c552c func_020c6ff8
#define func_020ca0a8 func_020cbb74
#define func_020ca2ac func_020cbd78
#define func_020ca4b4 func_020cbf80
#define func_020ca430 func_020cbefc

#define data_0210cf78 data_0210cc30
#endif

extern "C"
{
    // transform 3-dimensional vector (treated as 4D in usual way) by 4x3 matrix
    void func_020c2034(const fix32_t* vec, const fix32_t* mat, fix32_t* outVec);

    // reduce 4x4 matrix to 4x3 matrix
    void func_020c2208(const fix32_t* in, fix32_t* out);

    // get high precision division result
    int64_t func_020c2c38();

    // prime hardware divider to calculate fixed point representation of 2^32/x
    void func_020c2c94(fix32_t);

    // reset various geometry registers
    void func_020c51dc();

    int func_020c54fc(Matrix4x4*); // try to read clip matrix, -1 on failure
    int func_020c552c(Matrix3x3*); // try to read vector result matrix, -1 on failure

    void func_020ca0a8(int dmaChannel, const void* source, unsigned int length,
        DMACompletionCallback onCompletion, int callbackUserdata);
    void func_020ca2ac(int dmaChannel, const void* source, unsigned int length,
        DMACompletionCallback onCompletion, int callbackUserdata);

    // inverse memcpy doing 32 bytes at a time, assumes 4 byte alignment
    void func_020ca4b4(const void*, void*, unsigned);
    // repeated u32 memwrite, shifts source ptr along but keeps writing
    // to the same destination
    void func_020ca430(const void* src, volatile void* dst, unsigned int byteCount);

}

void WaitForFifoProcessingDone();

struct QueuedData
{
    uint32_t amountQueued;
    uint32_t data[0xc0];
};

struct Struct_0210cf78
{
    QueuedData* pQueuedData;
    volatile int fifoProcessingFlag; // 1 = processing, 0 = done
    int unknown_8;
} extern data_0210cf78;

extern "C" void SendQueuedDataToGeometryFifo()
{
    if (data_0210cf78.fifoProcessingFlag)
        WaitForFifoProcessingDone();

    if (data_0210cf78.pQueuedData != NULL && data_0210cf78.pQueuedData->amountQueued != 0)
    {
        func_020ca430(data_0210cf78.pQueuedData->data, &GXFIFO, data_0210cf78.pQueuedData->amountQueued * 4);
        data_0210cf78.pQueuedData->amountQueued = 0;
    }
}

// can be made static. 020b69f4
void WaitForFifoProcessingDone()
{
    while (data_0210cf78.fifoProcessingFlag) {}
}

// can be made static. 020b6a0c
void OnGeometryFifoProcessingDone(int userdata)
{
    volatile int* pFlag = reinterpret_cast<volatile int*>(userdata);
    *pFlag = 0;
}

extern "C" void SendRawDataToGeometryFifo(const uint32_t* data, unsigned int numBytes)
{
    if (numBytes < 256 || data_020f2270 == -1)
        SubmitCommandToGeometryFifo(data[0], &data[1], (numBytes / 4) - 1);
    else
    {
        SendQueuedDataToGeometryFifo();
        data_0210cf78.fifoProcessingFlag = 1;
        if (data_0210cf78.unknown_8)
            func_020ca2ac(data_020f2270, data, numBytes, &OnGeometryFifoProcessingDone,
                reinterpret_cast<int>(&data_0210cf78.fifoProcessingFlag));
        else
            func_020ca0a8(data_020f2270, data, numBytes, &OnGeometryFifoProcessingDone,
                reinterpret_cast<int>(&data_0210cf78.fifoProcessingFlag));
    }
}

extern "C" void SubmitCommandToGeometryFifo(int command, const uint32_t* params, unsigned int numParams)
{
    if (data_0210cf78.pQueuedData != NULL)
    {
        if (data_0210cf78.fifoProcessingFlag)
        {
            uint32_t priorSize = data_0210cf78.pQueuedData->amountQueued;
            if (priorSize + 1 + numParams <= 0xc0)
            {
                data_0210cf78.pQueuedData->amountQueued = priorSize + 1;
                data_0210cf78.pQueuedData->data[priorSize] = command;
                if (numParams != 0)
                {
                    func_020ca4b4(params,
                        &data_0210cf78.pQueuedData->data[data_0210cf78.pQueuedData->amountQueued],
                        numParams * 4);
                    data_0210cf78.pQueuedData->amountQueued += numParams;
                }
                return;
            }
        }

        if (data_0210cf78.pQueuedData->amountQueued != 0)
            SendQueuedDataToGeometryFifo();
        else
        {
            if (data_0210cf78.fifoProcessingFlag)
                WaitForFifoProcessingDone();
        }
    }
    else
    {
        if (data_0210cf78.fifoProcessingFlag)
            WaitForFifoProcessingDone();
    }

    GXFIFO = command;
    func_020ca430(params, &GXFIFO, numParams * 4);
}

void GetCurrentPositionAndDirectionMatrices(Matrix4x3* position, Matrix3x3* direction)
{
    SendQueuedDataToGeometryFifo();
    GXFIFO_MATRIX_MODE = 0; // projection
    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_MATRIX_IDENTITY = 0;

    if (position != NULL)
    {
        Matrix4x4 clipMatrix4x4;
        while (func_020c54fc(&clipMatrix4x4) != 0) {}
        Mat4x4_ConvertTo4x3(&clipMatrix4x4, position);
    }

    if (direction != NULL)
    {
        while (func_020c552c(direction) != 0) {}
    }

    GXFIFO_MATRIX_POP = 1;
    GXFIFO_MATRIX_MODE = 2; // return to position+direction mode
}

bool GetModelBonePositionAndDirectionMatrices(ModelRenderContext *context,
    Matrix4x3* outPos, Matrix3x3* outDir, unsigned int boneIndex)
{
    NSBXXInternalModel* model = context->internalModel_;
    NSBXXNameList* boneList = &model->boneList_;
    NSBXXBoneMatrix* boneMatrix = boneList->GetEntryFromu32Offset_v2<NSBXXBoneMatrix>(boneIndex);

    unsigned int stackPos = ((unsigned int)boneMatrix->flags_ & 0xf800) >> 11;
    if (stackPos != 31)
    {
        uint32_t arg = stackPos;
        SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &arg, 1);
        if (outPos != NULL || outDir != NULL)
            GetCurrentPositionAndDirectionMatrices(outPos, outDir);

        return true;
    }
    return false;
}

void Finish3DRendering()
{
    func_020c51dc();
    RenderConfig::Reset();
    // set IRQ mode to 2: irq when fifo becomes empty
    GXSTATUS = GXSTATUS & ~0xc0000000 | 0x80000000;
}

int ConvertWorldToScreenCoordinates(const Vector3fix *world, int *outX, int *outY)
{
    int success;
    Vector3fix viewVector;

    Matrix4x4* proj = &data_0210a010.projectionMatrix;

    Mat4x3_ApplyToVector(world, &data_0210a010.viewMatrix, &viewVector);

    int64_t projectedW = (uint64_t)viewVector.x * proj->entries[3] +
        (int64_t)viewVector.y * proj->entries[7] +
        (int64_t)viewVector.z * proj->entries[11];

    fix32_QueueComputeReciprocal((fix32_t)(projectedW >> 12) + proj->entries[15]);

    int64_t productX = (uint64_t)viewVector.x * proj->entries[0] +
        (int64_t)viewVector.y * proj->entries[4] +
        (int64_t)viewVector.z * proj->entries[8];

    fix32_t reducedX = (fix32_t)(productX >> 12) + proj->entries[12];

    int64_t productY = (uint64_t)viewVector.x * proj->entries[1] +
        (int64_t)viewVector.y * proj->entries[5] +
        (int64_t)viewVector.z * proj->entries[9];

    fix32_t reducedY = (fix32_t)(productY >> 12) + proj->entries[13];

    int64_t largeInvW = GetHardwareDividerResult();

    // Compute x/w and y/w using a 32-bit right shift, then transform via
    // u -> (1 + u)/2 to adjust the range from (-1, 1) to (0, 1)
    fix32_t normalizedX = ((fix32_t)((largeInvW * reducedX + (1u << 31)) >> 32) + 0x1000) / 2;
    fix32_t normalizedY = ((fix32_t)((largeInvW * reducedY + (1u << 31)) >> 32) + 0x1000) / 2;

    success = 0;

    if (normalizedX < 0 || normalizedY < 0 || normalizedX > 0x1000 || normalizedY > 0x1000)
        success = -1;

    int left, top, right, bottom;

    RenderConfig::GetViewport(&left, &top, &right, &bottom);

    int width = right - left;
    int height = bottom - top;

    *outX = left + ((normalizedX * width + 0x800) >> 12);
    *outY = (191 - top) - ((normalizedY * height + 0x800) >> 12);

    return success;
}