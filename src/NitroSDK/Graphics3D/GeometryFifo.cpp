#include "NitroSDK/Graphics.h"
#include "NitroSDK/Graphics3D/Graphics3D.h"
#include "NitroSDK/DMA.h"
#include <globaldefs.h>

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
#endif

extern "C"
{
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

static void WaitForFifoProcessingDone();

struct QueuedData
{
    uint32_t amountQueued;
    uint32_t data[0xc0];
};

static volatile CBool s_isFifoProcessing;
static QueuedData* s_queuedData;
static CBool s_unknown;

void Nitro3D_FlushFifoQueue()
{
    if (s_isFifoProcessing)
        WaitForFifoProcessingDone();

    if (s_queuedData != NULL && s_queuedData->amountQueued != 0)
    {
        func_020ca430(s_queuedData->data, &GXFIFO, s_queuedData->amountQueued * 4);
        s_queuedData->amountQueued = 0;
    }
}

static void WaitForFifoProcessingDone()
{
    while (s_isFifoProcessing) {}
}

static void OnGeometryFifoProcessingDone(int userdata)
{
    volatile int* pFlag = reinterpret_cast<volatile int*>(userdata);
    *pFlag = 0;
}

void Nitro3Di_SendRawDataToFifo(const uint32_t* data, unsigned int numBytes)
{
    if (numBytes < 256 || data_020f2270 == -1)
        Nitro3Di_SendFifoCommand(data[0], &data[1], (numBytes / 4) - 1);
    else
    {
        Nitro3D_FlushFifoQueue();
        s_isFifoProcessing = 1;
        if (s_unknown)
            func_020ca2ac(data_020f2270, data, numBytes, &OnGeometryFifoProcessingDone,
                reinterpret_cast<int>(&s_isFifoProcessing));
        else
            func_020ca0a8(data_020f2270, data, numBytes, &OnGeometryFifoProcessingDone,
                reinterpret_cast<int>(&s_isFifoProcessing));
    }
}

extern "C" void Nitro3Di_SendFifoCommand(int command, const uint32_t* params, unsigned int numParams)
{
    if (s_queuedData != NULL)
    {
        if (s_isFifoProcessing)
        {
            uint32_t priorSize = s_queuedData->amountQueued;
            if (priorSize + 1 + numParams <= 0xc0)
            {
                s_queuedData->amountQueued = priorSize + 1;
                s_queuedData->data[priorSize] = command;
                if (numParams != 0)
                {
                    func_020ca4b4(params,
                        &s_queuedData->data[s_queuedData->amountQueued],
                        numParams * 4);
                    s_queuedData->amountQueued += numParams;
                }
                return;
            }
        }

        if (s_queuedData->amountQueued != 0)
            Nitro3D_FlushFifoQueue();
        else
        {
            if (s_isFifoProcessing)
                WaitForFifoProcessingDone();
        }
    }
    else
    {
        if (s_isFifoProcessing)
            WaitForFifoProcessingDone();
    }

    GXFIFO = command;
    func_020ca430(params, &GXFIFO, numParams * 4);
}

void Nitro3D_GetCurrentPosDirMatrices(Matrix4x3* position, Matrix3x3* direction)
{
    Nitro3D_FlushFifoQueue();
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

bool Nitro3D_GetBonePosDirMatrices(Nitro3DRenderContext *context,
    Matrix4x3* outPos, Matrix3x3* outDir, unsigned int boneIndex)
{
    Nitro3DModel* model = context->model;
    Nitro3DNameList* boneList = &model->boneList;
    Nitro3DBoneMatrix* boneMatrix = boneList->GetEntryFromu32Offset<Nitro3DBoneMatrix>(boneIndex);

    unsigned int stackPos = ((unsigned int)boneMatrix->flags & 0xf800) >> 11;
    if (stackPos != 31)
    {
        uint32_t arg = stackPos;
        Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &arg, 1);
        if (outPos != NULL || outDir != NULL)
            Nitro3D_GetCurrentPosDirMatrices(outPos, outDir);

        return true;
    }
    return false;
}

void Nitro3D_FinishRender()
{
    func_020c51dc();
    Nitro3DRenderConfig::Reset();
    // set IRQ mode to 2: irq when fifo becomes empty
    GXSTATUS = GXSTATUS & ~0xc0000000 | 0x80000000;
}

int Nitro3D_WorldToScreen(const Vector3fix *world, int *outX, int *outY)
{
    int success;
    Vector3fix viewVector;

    Matrix4x4* proj = &g_nitro3DRenderConfig.projectionMatrix;

    Mat4x3_ApplyToVector(world, &g_nitro3DRenderConfig.viewMatrix, &viewVector);

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

    Nitro3DRenderConfig::GetViewport(&left, &top, &right, &bottom);

    int width = right - left;
    int height = bottom - top;

    *outX = left + ((normalizedX * width + 0x800) >> 12);
    *outY = (191 - top) - ((normalizedY * height + 0x800) >> 12);

    return success;
}