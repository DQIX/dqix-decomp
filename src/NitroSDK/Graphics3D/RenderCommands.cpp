#include "NitroSDK/Graphics3D/Internal.h"

static void MaterialBindProc(Nitro3DRenderCommandHandler* handler, int modifier, Nitro3DMaterial* material, unsigned int idx);
static void MeshDrawProc(Nitro3DRenderCommandHandler* handler, int modifier, Nitro3DMesh* mesh, unsigned int meshIdx);

static void RenderCommand_0(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_1(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_2(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_3(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_4(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_5(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_6(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_7(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_8(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_9(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_10(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_11(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_12(Nitro3DRenderCommandHandler* handler, int modifier);
static void RenderCommand_13(Nitro3DRenderCommandHandler* handler, int modifier);

struct BillboardFIFOData {
    // is always (12, 10, 17, 1b) = (pop matrix, matrix mode, load 4x3, scale)
    // which can be merged with subsequent data as a single command to the GXFIFO.
    uint32_t commandsCombined;
    // always 1
    uint32_t popMatrixParameter;
    // always 2 (position + vector?)
    uint32_t matrixModeParameter;
    Matrix4x3 rotationTranslation;
    Vector3fix scaling;
};

static void (*s_renderCommands[32])(Nitro3DRenderCommandHandler*, int) =
{
    &RenderCommand_0, &RenderCommand_1, &RenderCommand_2, 
    &RenderCommand_3, &RenderCommand_4, &RenderCommand_5,
    &RenderCommand_6, &RenderCommand_7, &RenderCommand_8,
    &RenderCommand_9, &RenderCommand_10, &RenderCommand_11,
    &RenderCommand_12, &RenderCommand_13
};

static void (*s_materialBindProcs[])(Nitro3DRenderCommandHandler*, int modifier, Nitro3DMaterial*, unsigned int idx) =
{
    &MaterialBindProc, NULL, NULL, NULL
};
static MaterialRenderData s_boundMaterialStorage[64];

static void (*s_meshDrawProcs[])(Nitro3DRenderCommandHandler*, int modifier, Nitro3DMesh*, unsigned int idx) =
{
    &MeshDrawProc, NULL, NULL, NULL
};

Nitro3DRenderCommandHandler* g_nitro3DActiveRenderCommandHandler;

static BillboardFIFOData s_freeBillboardFifo =
{
    COMBINE_GXFIFO_COMMANDS4(GXFifoCommand_PopMatrix,
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x3,
        GXFifoCommand_ScaleMatrix),
    1, // Pop matrix argument
    2, // matrix mode 2 (position + vector)
    { 1 << 12, 0, 0, //
      0, 1 << 12, 0, // rotation part of matrix 
      0, 0, 1 << 12, //
      0, 0, 0 }, // translation
    { 0, 0, 0 } // scale
};

static BillboardFIFOData s_axisBillboardFifo =
{
    COMBINE_GXFIFO_COMMANDS4(GXFifoCommand_PopMatrix,
        GXFifoCommand_SetMatrixMode,
        GXFifoCommand_LoadMat4x3,
        GXFifoCommand_ScaleMatrix),
    1, // Pop matrix argument
    2, // matrix mode 2 (position + vector)
    { 1 << 12, 0, 0, //
      0, 1 << 12, 0, // rotation part of matrix 
      0, 0, 1 << 12, //
      0, 0, 0 }, // translation
    { 0, 0, 0 } // scale
};

Nitro3DSkinningEquationBuffer g_nitro3DSkinningEquationBuffer;

static unsigned int s_command12TexImageParamsFifo[] = 
{
    GXFifoCommand_SetTexImageParams, 0
};

static unsigned int s_command13TexImageParamsFifo[] = 
{
    GXFifoCommand_SetTexImageParams, 0
};
static Matrix4x4 s_command13TextureMatrix = 
{
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 16 << 12, 0,
    0, 0, 0, 16 << 12
};

static inline int TestBitfieldBit(const unsigned int* bitfield, unsigned int idx)
{
    return bitfield[idx >> 5] & (1 << (idx & 31));
}

static inline int SetBitfieldBit(unsigned int* bitfield, unsigned int idx)
{
    bitfield[idx >> 5] |= (1 << (idx & 31));
}

static void ExecuteRenderCommands(Nitro3DRenderCommandHandler* handler)
{
    do
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        uint8_t opcode = handler->instructionPointer[0];
        s_renderCommands[opcode & 0x1f](handler, opcode & 0xe0);
    } while (!(handler->flags & (1 << RCH_FLAG_5)));
}

static void SetUpRenderCommandHandler(Nitro3DRenderCommandHandler* handler, Nitro3DRenderContext* modelData)
{
    func_020ca458(0, handler, sizeof(Nitro3DRenderCommandHandler));
    handler->boneMatrixBitfield[0] = 1;
    handler->flags = 1;

    uint8_t* commands = modelData->renderCommandList;
    if (commands == NULL)
        commands = (uint8_t*)modelData->model + modelData->model->renderCommandsOffset;
    *(uint8_t* volatile*)&handler->instructionPointer = commands;
    
    handler->context = modelData;

    if (modelData->model != NULL)
        handler->boneList = &modelData->model->boneList;
    else
        handler->boneList = NULL;

    handler->modelMaterials = modelData->model->GetMaterialData();
    handler->meshList = modelData->model->GetMeshList();
    handler->pfnBoneMatrixRenderDataScalePopulate = g_nitro3DPopulateBoneMatrixScalingProcs[modelData->model->boneScalingMode];
    handler->pfnBoneMatrixRenderDataSubmit = g_nitro3DSubmitBoneMatrixProcs[modelData->model->boneScalingMode];
    handler->pfnTextureMatrixCreate = g_nitro3DLoadMaterialTextureMatrixProcs[modelData->model->materialCallbackType];

    handler->upScale = modelData->model->upScale;
    handler->downScale = modelData->model->downScale;

    if (modelData->renderCommandHook != NULL && modelData->renderCommandHookCommandID < 32)
    {
        handler->hooks[modelData->renderCommandHookCommandID] = modelData->renderCommandHook;
        handler->hookStages[modelData->renderCommandHookCommandID] = modelData->renderCommandHookStage;
    }

    if (modelData->flags & 1)
        handler->flags |= (1 << RCH_FLAG_7);
    if (modelData->flags & 2)
        handler->flags |= (1 << RCH_FLAG_8);
    if (modelData->flags & 4)
        handler->flags |= (1 << RCH_FLAG_9);
    if (modelData->flags & 8)
        handler->flags |= (1 << RCH_FLAG_10);

    if (modelData->preRenderCallback != NULL)
    {
        void (*proc)(Nitro3DRenderCommandHandler*) = (void(*)(Nitro3DRenderCommandHandler*))modelData->preRenderCallback;
        proc(handler);
    }

    ExecuteRenderCommands(handler);
    modelData->flags &= ~1;
}

static void PopulateBitfieldFromAnimData(unsigned int* bitfield, Nitro3DAnimation* anim)
{
    while (anim != NULL)
    {
        for (int i = 0; i < anim->numEntries; i++)
        {
            if (anim->entries[i] & 0x100)
                bitfield[i >> 5] |= 1 << (i & 0x1f);
        }
        anim = anim->pNext;
    }
}

void Nitro3D_Render(Nitro3DRenderContext* renderData)
{
    Nitro3DRenderCommandHandler handler;
    
    if ((renderData->flags & 0x10) == 0x10)
    {
        func_020ca3ec(0, renderData->animatedMaterials, sizeof(renderData->animatedMaterials));
        func_020ca3ec(0, renderData->animatedBoneMatrices, sizeof(renderData->animatedBoneMatrices));
        func_020ca3ec(0, renderData->animatedVisibilityConditions, sizeof(renderData->animatedVisibilityConditions));
        if (renderData->materialAnimations != NULL)
            PopulateBitfieldFromAnimData(renderData->animatedMaterials, renderData->materialAnimations);
        if (renderData->jointAnimations != NULL)
            PopulateBitfieldFromAnimData(renderData->animatedBoneMatrices, renderData->jointAnimations);
        if (renderData->visibilityAnimations != NULL)
            PopulateBitfieldFromAnimData(renderData->animatedVisibilityConditions, renderData->visibilityAnimations);

        renderData->flags &= ~0x10;
    }

    if (g_nitro3DActiveRenderCommandHandler != NULL)
    {
        SetUpRenderCommandHandler(g_nitro3DActiveRenderCommandHandler, renderData);
    }
    else
    {
        g_nitro3DActiveRenderCommandHandler = &handler;
        SetUpRenderCommandHandler(&handler, renderData);
        g_nitro3DActiveRenderCommandHandler = NULL;
    }
}

// nsbmd docs call this 'nop' though the callback still occurs
static void RenderCommand_0(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (handler->hooks[0] != NULL)
        handler->hooks[0](handler);
    handler->instructionPointer++;
}

// signal end of render commands
static void RenderCommand_1(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (handler->hooks[1] != NULL)
        handler->hooks[1](handler);
    handler->flags |= (1 << RCH_FLAG_5);
}

// apply visibility condition to be used by animation
// params: condition index, default value (lowest bit used)
static void RenderCommand_2(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags & (1 << RCH_FLAG_9)))
    {
        unsigned int conditionIdx = handler->instructionPointer[1];
        handler->command2Arg1 = conditionIdx;
        handler->flags |= (1 << RCH_FLAG_2);
        handler->pCommand2Word = &handler->scratchCommand2Word;

        int callbackStage = (handler->hooks[2] != NULL) ? handler->hookStages[2] : 0;

        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[2](handler);
            callbackStage = (handler->hooks[2] != NULL) ? handler->hookStages[2] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);   
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            Nitro3DRenderContext* modelData = handler->context;
            if (modelData->visibilityAnimations == NULL ||
                !TestBitfieldBit(modelData->animatedVisibilityConditions, conditionIdx) ||
                !modelData->pfnProcessVisibilityAnimations(handler->pCommand2Word, modelData->visibilityAnimations, conditionIdx))
            {
                *handler->pCommand2Word = handler->instructionPointer[2] & 1;
            }
        }

        if (callbackStage == 2)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[2](handler);
            callbackStage = (handler->hooks[2] != NULL) ? handler->hookStages[2] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            if (*handler->pCommand2Word != 0)
                handler->flags |= (1 << RCH_FLAG_0);
            else
                handler->flags &= ~(1 << RCH_FLAG_0);
        }

        if (callbackStage == 3)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[2](handler);
        }
    }
    handler->instructionPointer += 3;
}

// load matrix from stack
// parameters: index to load from
static void RenderCommand_3(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags & (1 << RCH_FLAG_9)) && (handler->flags & (1 << RCH_FLAG_0)))
    {
        int callbackStage = (handler->hooks[3] != NULL) ? handler->hookStages[3] : 0;

        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[3](handler);
            callbackStage = (handler->hooks[3] != NULL) ? handler->hookStages[3] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            uint32_t stackPos = handler->instructionPointer[1];
            if (!(handler->flags & (1 << RCH_FLAG_8)))
                Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &stackPos, 1);
        }

        if (callbackStage == 3)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[3](handler);
        }
    }
    handler->instructionPointer += 2;
}

static void MaterialBindProc(Nitro3DRenderCommandHandler* handler, int modifier, Nitro3DMaterial* material, unsigned int idx)
{
    handler->boundMaterial = idx;
    handler->flags |= (1 << RCH_FLAG_3);
    handler->pMaterialRenderData = &handler->scratchMaterialRenderData;

    int callbackStage = (handler->hooks[4] != NULL) ? handler->hookStages[4] : 0;

    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[4](handler);
        callbackStage = (handler->hooks[4] != NULL) ? handler->hookStages[4] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0)
    {
        MaterialRenderData* targetData;
        MaterialRenderData* modelMaterialArray = handler->context->materialRenderDataArray;
        if (modelMaterialArray != NULL && !(handler->flags & (1 << RCH_FLAG_7)))
            targetData = &modelMaterialArray[idx];
        else if ((modifier == 0x20 || modifier == 0x40) && TestBitfieldBit(handler->materialBitfield, idx))
        {
            if (modelMaterialArray != NULL)
                targetData = &modelMaterialArray[idx];
            else
                targetData = &s_boundMaterialStorage[idx];
        }
        else
        {
            if (modelMaterialArray != NULL)
            {
                SetBitfieldBit(handler->materialBitfield, idx);
                targetData = &handler->context->materialRenderDataArray[idx];
            }
            else
            {
                if (!(modifier != 0x40))
                {
                    SetBitfieldBit(handler->materialBitfield, idx);
                    targetData = &s_boundMaterialStorage[idx];
                }
                else
                    targetData = &handler->scratchMaterialRenderData;
            }

            targetData->flags = 0;
            Nitro3DMaterial* lookupMaterial = handler->modelMaterials->GetMaterialByIndex(idx);
            if (lookupMaterial->flags & 0x20)
                targetData->flags |= 0x20;

            static const unsigned int maskLookup[] = 
            {
                0, 0x7fff, 0x7fff0000, 0x7fff7fff,
                0x8000, 0xffff, 0x7fff8000, 0x7fffffff
            };
            
            int difambMask = maskLookup[(material->flags >> 6) & 7];
            targetData->paramDIF_AMB = (g_nitro3DRenderConfig.diffuseAmbientArg & ~difambMask) |
                (material->paramDIF_AMB & difambMask);

            int speemiMask = maskLookup[(material->flags >> 9) & 7];
            targetData->paramSPE_EMI = (g_nitro3DRenderConfig.specularArg & ~speemiMask) |
                (material->paramSPE_EMI & speemiMask);

            targetData->paramPOLYGON_ATTR = (g_nitro3DRenderConfig.polygonAttrArg & ~material->maskPOLYGON_ATTR) |
                (material->paramPOLYGON_ATTR & material->maskPOLYGON_ATTR);
            targetData->paramTEXIMAGE_PARAMS = material->paramTEXIMAGE_PARAMS;
            targetData->texturePaletteBase = material->texturePaletteVRAMOffset;

            if (material->flags & 1)
            {
                intptr_t extensionDataAddress = (intptr_t)(material + 1);
                if (!(material->flags & 2))
                {
                    Nitro3DMaterial::ExtensionData_Bit1* extensionData = 
                        (Nitro3DMaterial::ExtensionData_Bit1*)extensionDataAddress;
                    targetData->extensionScaleX = extensionData->scaleX;
                    targetData->extensionScaleY = extensionData->scaleY;
                    extensionDataAddress += sizeof(*extensionData);
                }
                else
                    targetData->flags |= 1;

                if (!(material->flags & 4))
                {
                    Nitro3DMaterial::ExtensionData_Bit2* extensionData =
                        (Nitro3DMaterial::ExtensionData_Bit2*)extensionDataAddress;
                    targetData->rotationSine = extensionData->sine;
                    targetData->rotationCosine = extensionData->cosine;
                    extensionDataAddress += sizeof(*extensionData);
                }
                else
                    targetData->flags |= 2;
                
                if (!(material->flags & 8))
                {
                    Nitro3DMaterial::ExtensionData_Bit3* extensionData = 
                        (Nitro3DMaterial::ExtensionData_Bit3*)extensionDataAddress;
                    targetData->translateX = extensionData->translateX;
                    targetData->translateY = extensionData->translateY;
                }
                else 
                    targetData->flags |= 4;

                targetData->flags |= 8;
            }

            Nitro3DRenderContext* modelData = handler->context;
            if (modelData->materialAnimations != NULL && TestBitfieldBit(modelData->animatedMaterials, idx))
                modelData->pfnProcessMaterialAnimations(targetData, modelData->materialAnimations, idx);

            if (targetData->flags & (0x10 | 0x08))
            {
                targetData->materialWidth = material->width;
                targetData->materialHeight = material->height;
                targetData->materialScaleX = material->xScale;
                targetData->materialScaleY = material->yScale;
            }
        }      
        handler->pMaterialRenderData = targetData;
    }

    if (callbackStage == 2)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[4](handler);
        callbackStage = (handler->hooks[4] != NULL) ? handler->hookStages[4] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0)
    {
        MaterialRenderData* targetData = handler->pMaterialRenderData;
        if ((targetData->paramPOLYGON_ATTR & 0x1f0000) != 0) // alpha != 0
        {
            if (targetData->flags & 0x20)
                targetData->paramPOLYGON_ATTR &= ~0x1f0000;
            handler->flags &= ~(1 << RCH_FLAG_1);
            if (!(handler->flags & (1 << RCH_FLAG_8)))
            {
                uint32_t commands[7];
                commands[0] = COMBINE_GXFIFO_COMMANDS3(
                    GXFifoCommand_DiffuseAmbientReflect,
                    GXFifoCommand_SpecularReflectEmit,
                    GXFifoCommand_SetPolygonAttr);
                commands[1] = targetData->paramDIF_AMB;
                commands[2] = targetData->paramSPE_EMI;
                commands[3] = targetData->paramPOLYGON_ATTR;
                commands[4] = COMBINE_GXFIFO_COMMANDS2(
                    GXFifoCommand_SetTexImageParams,
                    GXFifoCommand_SetTexturePaletteBase);
                commands[5] = targetData->paramTEXIMAGE_PARAMS;
                commands[6] = targetData->texturePaletteBase;
                Nitro3Di_SendFifoCommand(commands[0], &commands[1], 6);

                if (targetData->flags & (0x10 | 0x08))
                    handler->pfnTextureMatrixCreate(targetData);
            }
        }
        else
            handler->flags |= (1 << RCH_FLAG_1);
    }

    if (callbackStage == 3)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[4](handler);
    }
}

// bind material
// parameters: index of material within the model's MaterialList
static void RenderCommand_4(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags & (1 << RCH_FLAG_9)))
    {
        int materialIdx = handler->instructionPointer[1];
        if ((handler->flags & (1 << RCH_FLAG_0)) || !(handler->flags & (1 << RCH_FLAG_3)) || materialIdx != handler->boundMaterial)
        {
            Nitro3DMaterial* material = handler->modelMaterials->GetMaterialByIndex(materialIdx);
            // MaterialBindProc
            s_materialBindProcs[material->bindMethod](handler, modifier, material, materialIdx);
        }
    }
    handler->instructionPointer += 2;
}

static void MeshDrawProc(Nitro3DRenderCommandHandler* handler, int modifier, Nitro3DMesh* mesh, unsigned int meshIdx)
{
    int callbackStage = (handler->hooks[5] != NULL) ? handler->hookStages[5] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[5](handler);
        callbackStage = (handler->hooks[5] != NULL) ? handler->hookStages[5] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0 && !(handler->flags & (1 << RCH_FLAG_8)))
        Nitro3Di_SendRawDataToFifo(mesh->GetGPUCommands(), mesh->gpuCommandsLength);

    if (callbackStage == 2)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[5](handler);
        callbackStage = (handler->hooks[5] != NULL) ? handler->hookStages[5] : 0;
    }

    if (callbackStage == 3)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[5](handler);
    }
}

// draw mesh
// parameters: index of mesh to draw
static void RenderCommand_5(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags & (1 << RCH_FLAG_9)) && (handler->flags & (1 << RCH_FLAG_0)) && !(handler->flags & (1 << RCH_FLAG_1)))
    {
        int meshIdx = handler->instructionPointer[1];
        Nitro3DNameList* meshList = handler->meshList;
        Nitro3DMesh* mesh = meshList->GetEntryFromu32Offset<Nitro3DMesh>(meshIdx);
        s_meshDrawProcs[mesh->drawMethod](handler, modifier, mesh, meshIdx);
    }
    handler->instructionPointer += 2;
}

// multiply current matrix by bone matrix (on the left)
// parameters: 
// 1) index of bone matrix
// 2) index of parent bone's bone matrix (needed for certain scaling operations)
// 3) unknown, seemingly unused
// if modifier & 0x20: stack position to store result at the end
// if modifier & 0x40: stack position of matrix to retrieve at the start, i.e. 
// you'll retrieve a matrix off the stack and pre-multiply that by the bone matrix
static void RenderCommand_6(Nitro3DRenderCommandHandler* handler, int modifier)
{
    int boneIdx = handler->instructionPointer[1];
    int numBytesConsumed = 4; // by default it's opcode + 3 args
    handler->currentBoneMatrix = boneIdx;
    handler->flags |= (1 << RCH_FLAG_4);
    if (handler->flags & (1 << RCH_FLAG_10))
    {
        if (modifier == 0x40 || modifier == 0x60)
            numBytesConsumed++;
        if (modifier == 0x20 || modifier == 0x60)
        {
            numBytesConsumed++;
            if (!(handler->flags & (1 << RCH_FLAG_8)))
            {
                uint32_t retrievalPosition = handler->instructionPointer[4];
                Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
            }
        }
        handler->instructionPointer += numBytesConsumed;
        return;
    }

    // if the 0x40 bit is set...
    if (modifier == 0x40 || modifier == 0x60)
    {
        // the presence of the 0x20 bit means we have to look one further
        uint8_t* instructionPtr = handler->instructionPointer;
        uint32_t retrievalPosition;
        if (modifier == 0x40) 
            retrievalPosition = instructionPtr[4];
        else 
            retrievalPosition = instructionPtr[5];
        numBytesConsumed++;
        if (!(handler->flags & (1 << RCH_FLAG_8)))
            Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
    }

    handler->pBoneMatrixRenderData = &handler->scratchBoneMatrixRenderData;
    int callbackStage = (handler->hooks[6] != NULL) ? handler->hookStages[6] : 0;

    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[6](handler);
        callbackStage = (handler->hooks[6] != NULL) ? handler->hookStages[6] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0)
    {
        BoneMatrixRenderData* targetData;
        BoneMatrixRenderData* modelBoneArray = handler->context->boneMatrixRenderDataArray;

        bool bit7clear;
        if (modelBoneArray != NULL)
        {
            targetData = &modelBoneArray[boneIdx];
            bit7clear = !(handler->flags & (1 << RCH_FLAG_7));
        }
        else
        {
            targetData = &handler->scratchBoneMatrixRenderData;
            bit7clear = false;
        }

        if (!bit7clear)
        {
            targetData->flags = 0;
            if (handler->context->jointAnimations == NULL ||
                !handler->context->pfnProcessJointAnimations(targetData, handler->context->jointAnimations, boneIdx))
            {
                Nitro3DBoneMatrix* boneMatrix = handler->boneList->GetEntryFromu32Offset<Nitro3DBoneMatrix>(boneIdx);
                intptr_t boneMatrixExtraPtr = (intptr_t)(boneMatrix + 1);
                if ((boneMatrix->flags & 1)) // has no translation component
                    targetData->flags |= 4;
                else
                {
                    Nitro3DBoneMatrix::Translation* translation = 
                        (Nitro3DBoneMatrix::Translation*)boneMatrixExtraPtr;
                    targetData->translate.x = translation->x;
                    targetData->translate.y = translation->y;
                    targetData->translate.z = translation->z;
                    boneMatrixExtraPtr += sizeof(*translation);
                }

                if (boneMatrix->flags & 2) // does not have any kind of rotation
                    targetData->flags |= 2;
                else if (boneMatrix->flags & 8) // use pivot matrix
                {
                    int form = (boneMatrix->flags & 0xf0) >> 4;
                    Nitro3DBoneMatrix::PivotMatrixData* pivot = 
                        (Nitro3DBoneMatrix::PivotMatrixData*)boneMatrixExtraPtr;
                    fix32_t entryA = pivot->a;
                    fix32_t entryB = pivot->b;
                    func_020ca7d0(&targetData->rotationMatrix);
                    
                    fix32_t unit;
                    if (boneMatrix->flags & 0x100)
                        unit = 0xfffff000; // -1.0
                    else
                        unit = 0x1000; // +1.0

                    targetData->rotationMatrix.entries[form] = unit;

                    struct { uint8_t a, b, c, d; } static const indexLookup[] = 
                    {
                        { 4, 5, 7, 8 },
                        { 3, 5, 6, 8 },
                        { 3, 4, 6, 7 },
                        { 1, 2, 7, 8 },
                        { 0, 2, 6, 8 },
                        { 0, 1, 6, 7 },
                        { 1, 2, 4, 5 },
                        { 0, 2, 3, 5 },
                        { 0, 1, 3, 4 }
                    };

                    uint8_t indexA = indexLookup[form].a;
                    uint8_t indexB = indexLookup[form].b;

                    targetData->rotationMatrix.entries[indexA] = entryA;
                    targetData->rotationMatrix.entries[indexB] = entryB;

                    fix32_t entryC = (boneMatrix->flags & 0x200) ? -entryB : entryB;
                    uint8_t indexC = indexLookup[form].c;
                    targetData->rotationMatrix.entries[indexC] = entryC;

                    fix32_t entryD = (boneMatrix->flags & 0x400) ? -entryA : entryA;
                    uint8_t indexD = indexLookup[form].d;
                    targetData->rotationMatrix.entries[indexD] = entryD;
                    boneMatrixExtraPtr += sizeof(*pivot);
                }
                else // use specified matrix
                {
                    // casting from fix16_t to fix32_t, this is okay because
                    // both have 12 bits after the point
                    targetData->rotationMatrix.entries[0] = boneMatrix->m_11;
                    Nitro3DBoneMatrix::RotationMatrixData* rotation = 
                        (Nitro3DBoneMatrix::RotationMatrixData*)boneMatrixExtraPtr;
                    targetData->rotationMatrix.entries[1] = rotation->entries[0];
                    targetData->rotationMatrix.entries[2] = rotation->entries[1];
                    targetData->rotationMatrix.entries[3] = rotation->entries[2];
                    targetData->rotationMatrix.entries[4] = rotation->entries[3];
                    targetData->rotationMatrix.entries[5] = rotation->entries[4];
                    targetData->rotationMatrix.entries[6] = rotation->entries[5];
                    targetData->rotationMatrix.entries[7] = rotation->entries[6];
                    targetData->rotationMatrix.entries[8] = rotation->entries[7];
                    boneMatrixExtraPtr += sizeof(*rotation);
                }

                handler->pfnBoneMatrixRenderDataScalePopulate(targetData,
                    (Nitro3DBoneMatrix::Scaling*)boneMatrixExtraPtr,
                    handler->instructionPointer, boneMatrix->flags);
            }
        }
        handler->pBoneMatrixRenderData = targetData;
    }

    if (callbackStage == 2)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[6](handler);
        callbackStage = (handler->hooks[6] != NULL) ? handler->hookStages[6] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;
    
    if (flagbit6 == 0 && !(handler->flags & (1 << RCH_FLAG_8)))
    {
        handler->pfnBoneMatrixRenderDataSubmit(handler->pBoneMatrixRenderData);
    }

    handler->pBoneMatrixRenderData = NULL;
    if (callbackStage == 3)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[6](handler);
        callbackStage = (handler->hooks[6] != NULL) ? handler->hookStages[6] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (modifier == 0x20 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (flagbit6 == 0 && !(handler->flags & (1 << RCH_FLAG_8)))
        {
            uint32_t storagePosition = handler->instructionPointer[4];
            Nitro3Di_SendFifoCommand(GXFifoCommand_StoreMatrix, &storagePosition, 1);
        }
    }

    handler->instructionPointer += numBytesConsumed;
}

// seems to be used for free billboarding, i.e. making stuff turn in any
// direction to face the camera. Noticeable with evac animation.
// parameters:
// 1) ???
// if modifier & 0x20: stack position to store result at the end
// if modifier & 0x40: stack position of matrix to retrieve at the start, i.e. 
// you'll retrieve a matrix off the stack and pre-multiply that by the bone matrix
static void RenderCommand_7(Nitro3DRenderCommandHandler* handler, int modifier)
{
    int numBytesConsumed = 2;
    Vector3fix* pTranslation = &s_freeBillboardFifo.rotationTranslation.translation;
    Vector3fix* pScaling = &s_freeBillboardFifo.scaling;
    if (handler->flags & (1 << RCH_FLAG_9))
    {
        if (modifier == 0x40 || modifier == 0x60)
            numBytesConsumed++;
        if (modifier == 0x20 || modifier == 0x60)
            numBytesConsumed++;
        handler->instructionPointer += numBytesConsumed;
        return;
    }

    if (modifier == 0x40 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (!(handler->flags & (1 << RCH_FLAG_8)))
        {
            uint8_t* instructionPtr = handler->instructionPointer;
            uint32_t retrievalPosition;
            if (modifier == 0x40)
                retrievalPosition = instructionPtr[2];
            else
                retrievalPosition = instructionPtr[3];
            Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
        }
    }

    int callbackStage = (handler->hooks[7] != NULL) ? handler->hookStages[7] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[7](handler);
        callbackStage = (handler->hooks[7] != NULL) ? handler->hookStages[7] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (!(handler->flags & (1 << RCH_FLAG_8)) && flagbit6 == 0)
    {
        Nitro3D_FlushFifoQueue();
        GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_PushMatrix, GXFifoCommand_LoadMatIdentity);
        GXFIFO = 0; // matrix mode 0: projection
        GXFIFO = 0; // why is this here?
        Matrix4x4 clipMatrix;
        // get the current world * view matrix (we set projection matrix to identity, so
        // it doesn't contribute here)
        while (func_020c54fc(&clipMatrix) != 0) {}
        if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            const Matrix4x3* worldView4x3 = Nitro3DRenderConfig::GetCombinedWorldViewMatrix();
            Matrix4x4 worldView;
            Mat4x3_ConvertTo4x4(worldView4x3, &worldView);
            Mat4x4_Multiply(&clipMatrix, &worldView, &clipMatrix);
        }
        else if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            Matrix4x4 view4x4;
            Mat4x3_ConvertTo4x4(&g_nitro3DRenderConfig.viewMatrix, &view4x4);
            Mat4x4_Multiply(&clipMatrix, &view4x4, &clipMatrix);
        }

        // store 4th row of matrix
        pTranslation->x = clipMatrix.entries[12];
        pTranslation->y = clipMatrix.entries[13];
        pTranslation->z = clipMatrix.entries[14];

        // store lengths of first 3 rows
        pScaling->x = Vector3fix_Length(&clipMatrix.rows[0].xyz);
        pScaling->y = Vector3fix_Length(&clipMatrix.rows[1].xyz);
        pScaling->z = Vector3fix_Length(&clipMatrix.rows[2].xyz);

        // note: if clipMatrix has a representation as (scaling) * (rotation) * (translation)
        // then the above is extracting the scaling and translation components

        if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&s_freeBillboardFifo.popMatrixParameter, &GXFIFO, 8); // pass pop / matrix mode commands
            // load the matrix returned by func_020b3a24(), inverse of func_020b39ec()
            func_020ca430(Nitro3DRenderConfig::GetInverseCombinedWorldViewMatrix(), &GXFIFO, 4*3 * sizeof(fix32_t));
            // the matrix to multiply by is a translation matrix, so we're composing
            // scaling with translation (remember the final operation represents
            // the first matrix transformation to carry out)
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&s_freeBillboardFifo.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&s_freeBillboardFifo.popMatrixParameter, &GXFIFO, 8);
            func_020ca430(Nitro3DRenderConfig::GetInverseViewMatrix(), &GXFIFO, 4*3 * sizeof(fix32_t));
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&s_freeBillboardFifo.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else
        {
            // send all the commands at once: pop the matrix, set matrix mode to 2,
            // then load a translation + scaling matrix
            func_020ca430(&s_freeBillboardFifo, &GXFIFO, sizeof(s_freeBillboardFifo));
        }

    }

    int newFlagbit6;
    if (callbackStage == 3)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[7](handler);
        newFlagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        newFlagbit6 = 0;

    if (modifier == 0x20 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (newFlagbit6 == 0 && !(handler->flags & (1 << RCH_FLAG_8)))
        {
            uint32_t storePosition = handler->instructionPointer[2];
            Nitro3Di_SendFifoCommand(GXFifoCommand_StoreMatrix, &storePosition, 1);
        }
    }
    handler->instructionPointer += numBytesConsumed;
}

// seems to be used for axis-aligned billboarding - noticeable on trees in
// Angel Falls & lights in cave grottos
static void RenderCommand_8(Nitro3DRenderCommandHandler* handler, int modifier)
{
    int numBytesConsumed = 2;
    Vector3fix* pTranslation = &s_axisBillboardFifo.rotationTranslation.translation;
    Vector3fix* pScaling = &s_axisBillboardFifo.scaling;
    Matrix3x3* pRotation = &s_axisBillboardFifo.rotationTranslation.rotation;
    if (handler->flags & (1 << RCH_FLAG_9))
    {
        if (modifier == 0x40 || modifier == 0x60)
            numBytesConsumed++;
        if (modifier == 0x20 || modifier == 0x60)
            numBytesConsumed++;
        handler->instructionPointer += numBytesConsumed;
        return;
    }

    if (modifier == 0x40 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (!(handler->flags & (1 << RCH_FLAG_8)))
        {
            uint8_t* instructionPtr = handler->instructionPointer;
            uint32_t retrievalPosition;
            if (modifier == 0x40)
                retrievalPosition = instructionPtr[2];
            else
                retrievalPosition = instructionPtr[3];
            Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
        }
    }

    int callbackStage = (handler->hooks[8] != NULL) ? handler->hookStages[8] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[8](handler);
        callbackStage = (handler->hooks[8] != NULL) ? handler->hookStages[8] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (!(handler->flags & (1 << RCH_FLAG_8)) && flagbit6 == 0)
    {
        Nitro3D_FlushFifoQueue();
        GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_PushMatrix, GXFifoCommand_LoadMatIdentity);
        GXFIFO = 0; // matrix mode 0: projection
        GXFIFO = 0; // why is this here?
        Matrix4x4 clipMatrix;
        while (func_020c54fc(&clipMatrix) != 0) {}
        if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            const Matrix4x3* worldView4x3 = Nitro3DRenderConfig::GetCombinedWorldViewMatrix();
            Matrix4x4 worldView;
            Mat4x3_ConvertTo4x4(worldView4x3, &worldView);
            Mat4x4_Multiply(&clipMatrix, &worldView, &clipMatrix);
        }
        else if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            Matrix4x4 view4x4;
            Mat4x3_ConvertTo4x4(&g_nitro3DRenderConfig.viewMatrix, &view4x4);
            Mat4x4_Multiply(&clipMatrix, &view4x4, &clipMatrix);
        }

        // store 4th row of matrix
        pTranslation->x = clipMatrix.entries[12];
        pTranslation->y = clipMatrix.entries[13];
        pTranslation->z = clipMatrix.entries[14];

        // store lengths of first 3 rows
        pScaling->x = Vector3fix_Length(&clipMatrix.rows[0].xyz);
        pScaling->y = Vector3fix_Length(&clipMatrix.rows[1].xyz);
        pScaling->z = Vector3fix_Length(&clipMatrix.rows[2].xyz);

        if (clipMatrix.rows[1].y != 0 || clipMatrix.rows[1].z != 0)
        {
            Vector3fix_Normalize(&clipMatrix.rows[1].xyz, &pRotation->rows[1]);
            // generate 3rd row via m_32 = -m_23, m_33 = m_22
            pRotation->entries[7] = -pRotation->entries[5];
            pRotation->entries[8] = pRotation->entries[4];
        }
        else
        {
            Vector3fix_Normalize(&clipMatrix.rows[2].xyz, &pRotation->rows[2]);
            // generate 2nd row
            pRotation->entries[5] = -pRotation->entries[7];
            pRotation->entries[4] = pRotation->entries[8];
        }
        
        // note: if clipMatrix has a representation as (scaling) * (rotation) * (translation)
        // then the above extracts the components, and reduces the rotation part to
        // only be a rotation in the x-axis

        if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&s_axisBillboardFifo.popMatrixParameter, &GXFIFO, 8); // pass pop / matrix mode commands
            func_020ca430(Nitro3DRenderConfig::GetInverseCombinedWorldViewMatrix(), &GXFIFO, 3 * 4 * sizeof(fix32_t));
            // the 4x3 matrix is a rotate-then-translate matrix. Recall that matrix multiplication
            // commands put the parameter on the inside, so we are actually ending 
            // up with (scale) -> (rotate then translate) -> (apply matrix from above)
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&s_axisBillboardFifo.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&s_axisBillboardFifo.popMatrixParameter, &GXFIFO, 8);
            func_020ca430(Nitro3DRenderConfig::GetInverseViewMatrix(), &GXFIFO, 4*3 * sizeof(fix32_t));
            // the 4x3 matrix is a rotate-then-translate matrix. Recall that matrix multiplication
            // commands put the parameter on the inside, so we are actually ending 
            // up with (scale) -> (rotate then translate) -> (apply matrix from above)
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&s_axisBillboardFifo.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else
        {
            // send all the commands at once: pop the matrix, set matrix mode to 2,
            // then pre-apply a translation, then pre-apply a scaling
            func_020ca430(&s_axisBillboardFifo, &GXFIFO, sizeof(s_axisBillboardFifo));
        }
    }

    int newFlagbit6;
    if (callbackStage == 3)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[8](handler);
        newFlagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        newFlagbit6 = 0;

    if (modifier == 0x20 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (newFlagbit6 == 0 && !(handler->flags & (1 << RCH_FLAG_8)))
        {
            uint32_t storePosition = handler->instructionPointer[2];
            Nitro3Di_SendFifoCommand(GXFifoCommand_StoreMatrix, &storePosition, 1);
        }
    }
    handler->instructionPointer += numBytesConsumed;
}

static void RenderCommand_9(Nitro3DRenderCommandHandler* handler, int modifier)
{
    int64_t weight = 0;
    Nitro3DModel* model = handler->context->model;    
    const Nitro3DInvBindMatrix* invBindMatrices = (Nitro3DInvBindMatrix*)((intptr_t)model + model->inverseBindsOffset);
    unsigned int numTerms = handler->instructionPointer[2];
    uint8_t* instructionPtr = handler->instructionPointer + 3;
    
    Nitro3DInvBindMatrix localCopy;
    func_020ca458(0, &localCopy, sizeof(Nitro3DInvBindMatrix));
    Nitro3D_FlushFifoQueue();

    GXFIFO_MATRIX_MODE = 0; // projection
    GXFIFO_MATRIX_STORE = 1;
    GXFIFO_MATRIX_IDENTITY = 0;
    GXFIFO_MATRIX_MODE = 2; // position+vector

    Matrix4x4* cachedPosMatrix;
    Matrix3x3* cachedDirMatrix;
    
    for (unsigned int termCounter = 0; termCounter < numTerms; termCounter++)
    {
        unsigned int inverseBindIndex = instructionPtr[1];
        
        int inBitfield = TestBitfieldBit(handler->invBindBitfield, inverseBindIndex);
        cachedPosMatrix = &g_nitro3DSkinningEquationBuffer.matrices[inverseBindIndex].mat4x4;
        if (!inBitfield)
        {
            SetBitfieldBit(handler->invBindBitfield, inverseBindIndex);
            // retrieve the local-to-world matrix from the stack
            GXFIFO_MATRIX_GET = instructionPtr[0];
            GXFIFO_MATRIX_MODE = 1; // position
            func_020c51a4(invBindMatrices[inverseBindIndex].mat4x3.entries);
        }

        if (termCounter != 0)
        {
            localCopy.mat3x3.entries[0] += (weight * cachedDirMatrix->entries[0]) >> 12;
            localCopy.mat3x3.entries[1] += (weight * cachedDirMatrix->entries[1]) >> 12;
            localCopy.mat3x3.entries[2] += (weight * cachedDirMatrix->entries[2]) >> 12;
            localCopy.mat3x3.entries[3] += (weight * cachedDirMatrix->entries[3]) >> 12;
            localCopy.mat3x3.entries[4] += (weight * cachedDirMatrix->entries[4]) >> 12;
            localCopy.mat3x3.entries[5] += (weight * cachedDirMatrix->entries[5]) >> 12;
            localCopy.mat3x3.entries[6] += (weight * cachedDirMatrix->entries[6]) >> 12;
            localCopy.mat3x3.entries[7] += (weight * cachedDirMatrix->entries[7]) >> 12;
            localCopy.mat3x3.entries[8] += (weight * cachedDirMatrix->entries[8]) >> 12;
        }
        if (!inBitfield)
        {
            while (func_020c54fc(cachedPosMatrix) != 0) {}
            GXFIFO_MATRIX_MODE = 2; // position+vector
            func_020c51c0(invBindMatrices[inverseBindIndex].mat3x3.entries);
        }
        weight = instructionPtr[2] << 4; // as int -> fixed point this is division by 256
        localCopy.mat4x3.entries[0] +=  (weight * cachedPosMatrix->entries[ 0]) >> 12;
        localCopy.mat4x3.entries[1] +=  (weight * cachedPosMatrix->entries[ 1]) >> 12;
        localCopy.mat4x3.entries[2] +=  (weight * cachedPosMatrix->entries[ 2]) >> 12;
        localCopy.mat4x3.entries[3] +=  (weight * cachedPosMatrix->entries[ 4]) >> 12;
        localCopy.mat4x3.entries[4] +=  (weight * cachedPosMatrix->entries[ 5]) >> 12;
        localCopy.mat4x3.entries[5] +=  (weight * cachedPosMatrix->entries[ 6]) >> 12;
        localCopy.mat4x3.entries[6] +=  (weight * cachedPosMatrix->entries[ 8]) >> 12;
        localCopy.mat4x3.entries[7] +=  (weight * cachedPosMatrix->entries[ 9]) >> 12;
        localCopy.mat4x3.entries[8] +=  (weight * cachedPosMatrix->entries[10]) >> 12;
        localCopy.mat4x3.entries[9] +=  (weight * cachedPosMatrix->entries[12]) >> 12;
        localCopy.mat4x3.entries[10] += (weight * cachedPosMatrix->entries[13]) >> 12;
        localCopy.mat4x3.entries[11] += (weight * cachedPosMatrix->entries[14]) >> 12;

        instructionPtr += 3;
        cachedDirMatrix = &g_nitro3DSkinningEquationBuffer.matrices[inverseBindIndex].mat3x3;
        if (!inBitfield)
        {
            while (func_020c552c(cachedDirMatrix) != 0) {}
        }
    }

    localCopy.mat3x3.entries[0] += (weight * cachedDirMatrix->entries[0]) >> 12;
    localCopy.mat3x3.entries[1] += (weight * cachedDirMatrix->entries[1]) >> 12;
    localCopy.mat3x3.entries[2] += (weight * cachedDirMatrix->entries[2]) >> 12;
    localCopy.mat3x3.entries[3] += (weight * cachedDirMatrix->entries[3]) >> 12;
    localCopy.mat3x3.entries[4] += (weight * cachedDirMatrix->entries[4]) >> 12;
    localCopy.mat3x3.entries[5] += (weight * cachedDirMatrix->entries[5]) >> 12;
    localCopy.mat3x3.entries[6] += (weight * cachedDirMatrix->entries[6]) >> 12;
    localCopy.mat3x3.entries[7] += (weight * cachedDirMatrix->entries[7]) >> 12;
    localCopy.mat3x3.entries[8] += (weight * cachedDirMatrix->entries[8]) >> 12;

    func_020c5188(localCopy.mat3x3.entries); // isn't this supposed to take a 3x4 matrix?
    GXFIFO_MATRIX_MODE = 1;
    func_020c5188(localCopy.mat4x3.entries);
    GXFIFO_MATRIX_MODE = 0;
    GXFIFO_MATRIX_GET = 1;
    GXFIFO_MATRIX_MODE = 2;
    GXFIFO_MATRIX_STORE = handler->instructionPointer[1];
    handler->instructionPointer += 3 + handler->instructionPointer[2] * 3;
}

// run raw gxfifo commands at the given offset and length.
// notably, the instruction pointer only advances past the offset and
// length data, so presumably this is stored past the end, like constants
// in assembly coming after the return statement
static void RenderCommand_10(Nitro3DRenderCommandHandler* handler, int modifier)
{
    int callbackStage = (handler->hooks[10] != NULL) ? handler->hookStages[10] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[10](handler);
        callbackStage = (handler->hooks[10] != NULL) ? handler->hookStages[10] : 0;
        flagbit6 = handler->flags & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (!(handler->flags & (1 << RCH_FLAG_8)) && flagbit6 == 0)
    {
        uint8_t* instructionPtr = handler->instructionPointer;
        uint32_t startOffset = (instructionPtr[1] | (instructionPtr[2] << 8) | (instructionPtr[3] << 16) | (instructionPtr[4] << 24));
        uint32_t length = (instructionPtr[5] | (instructionPtr[6] << 8) | (instructionPtr[7] << 16) | (instructionPtr[8] << 24));
        Nitro3Di_SendRawDataToFifo((uint32_t*)(instructionPtr + startOffset), length); 
    }

    if (callbackStage == 3)
    {
        handler->flags &= ~(1 << RCH_FLAG_6);
        handler->hooks[10](handler);
    }

    handler->instructionPointer += 9;
}

static void RenderCommand_11(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags & (1 << RCH_FLAG_8)) && !(handler->flags & (1 << RCH_FLAG_9)))
    {
        uint32_t scaleParams[3];
        if (modifier == 0)
            scaleParams[0] = scaleParams[1] = scaleParams[2] = handler->upScale;
        else
            scaleParams[0] = scaleParams[1] = scaleParams[2] = handler->downScale;

        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, scaleParams, 3);
    }
    handler->instructionPointer++;
}

// This instruction isn't explained in the nsbmd docs. Given the use of normals
// as inputs for texture coordinates and the same camera matrices showing up
// as in commands 7 and 8, this might be something like matcap lighting
// 2 parameters: 1) index of material in model's material list to use
// second parameter is unknown / seems to be unused
static void RenderCommand_12(Nitro3DRenderCommandHandler* handler, int modifier)
{
    Matrix3x3 directionMat3x3;
    if (!(handler->flags & (1 << RCH_FLAG_9)) && (handler->flags & (1 << RCH_FLAG_0)))
    {
        // if texcoord transformation mode != 2 (normal), set it to 2 and submit
        // the new params. In this mode, final texture coords to be used
        // are given by (normal) * TextureMatrix + (raw coords)
        // with raw coords being the argument to GXFifoCommand_SetTextureCoords
        if ((handler->pMaterialRenderData->paramTEXIMAGE_PARAMS & 0xc0000000) != 0x80000000)
        {
            handler->pMaterialRenderData->paramTEXIMAGE_PARAMS &= ~0xc0000000;
            handler->pMaterialRenderData->paramTEXIMAGE_PARAMS |= 0x80000000;
            s_command12TexImageParamsFifo[1] = handler->pMaterialRenderData->paramTEXIMAGE_PARAMS;
            Nitro3Di_SendFifoCommand(s_command12TexImageParamsFifo[0],
                &s_command12TexImageParamsFifo[1], 1);
        }
        // switch the matrix mode to texture
        uint32_t textureMatrixMode = 3;
        Nitro3Di_SendFifoCommand(GXFifoCommand_SetMatrixMode, &textureMatrixMode, 1);

        int callbackStage = (handler->hooks[12] != NULL) ? handler->hookStages[12] : 0;
        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[12](handler);
            callbackStage = (handler->hooks[12] != NULL) ? handler->hookStages[12] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;
            
        if (flagbit6 == 0)
        {
            unsigned int materialWidth = handler->pMaterialRenderData->materialWidth;
            unsigned int materialHeight = handler->pMaterialRenderData->materialHeight;

            uint32_t scaleParams[3];
            scaleParams[0] = materialWidth * 0x8000;
            scaleParams[1] = -materialHeight * 0x8000;
            scaleParams[2] = 0x10000;
            Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, scaleParams, 3);
            // Due to 4 bits after the fixed point, this * 8 is really a division by 2
            short xcoord = materialWidth * 8;
            short ycoord = materialHeight * 8;
            uint32_t texCoordPack = (unsigned short)xcoord | ((unsigned short)ycoord << 16);
            Nitro3Di_SendFifoCommand(GXFifoCommand_SetTextureCoords, &texCoordPack, 1);
        }

        if (callbackStage == 2)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[12](handler);
            callbackStage = (handler->hooks[12] != NULL) ? handler->hookStages[12] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            unsigned int materialIdx = handler->instructionPointer[1];
            Nitro3DMaterial* material = handler->modelMaterials->GetMaterialByIndex(materialIdx);
            if (material->flags & 0x2000)
            {
                intptr_t bit13data = (intptr_t)(material + 1);
                if (!(material->flags & 2))
                    bit13data += sizeof(Nitro3DMaterial::ExtensionData_Bit1);
                if (!(material->flags & 4))
                    bit13data += sizeof(Nitro3DMaterial::ExtensionData_Bit2);
                if (!(material->flags & 8))
                    bit13data += sizeof(Nitro3DMaterial::ExtensionData_Bit3);
                
                Nitro3DMaterial::ExtensionData_Bit13* ptr = (Nitro3DMaterial::ExtensionData_Bit13*)bit13data;
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x4, (uint32_t*)ptr->matrix.entries, 16);
            }
        }

        if (callbackStage == 3)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[12](handler);
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            uint32_t posVectorMode = 2;
            Nitro3Di_SendFifoCommand(GXFifoCommand_SetMatrixMode, &posVectorMode, 1);
            
            Nitro3D_GetCurrentPosDirMatrices(NULL, &directionMat3x3);
            uint32_t textureMode = 3;
            Nitro3Di_SendFifoCommand(GXFifoCommand_SetMatrixMode, &textureMode, 1);
            if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_0))
            {
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&g_nitro3DRenderConfig.viewMatrix.entries, 9);
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&g_nitro3DRenderConfig.objectRotationPosition.rotation.entries, 9);
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&directionMat3x3.entries, 9);
            }
            else if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_1))
            {
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&g_nitro3DRenderConfig.viewMatrix.entries, 9);
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&directionMat3x3.entries, 9);
            }
            else
            {
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&directionMat3x3.entries, 9);
            }
        }
        // switch back to position+vector mode
        uint32_t endMode = 2;
        Nitro3Di_SendFifoCommand(GXFifoCommand_SetMatrixMode, &endMode, 1);
    }
    handler->instructionPointer += 3;
}

// I have no idea what this is, but it's texture related. I couldn't find any
// use of it in the game.
// 2 parameters: 1) index of material in model's material list, 2) unknown/unused
static void RenderCommand_13(Nitro3DRenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags & (1 << RCH_FLAG_9)) && (handler->flags & (1 << RCH_FLAG_0)))
    {
        Matrix4x3 worldView;
        Nitro3D_GetCurrentPosDirMatrices(&worldView, NULL);

        // store onto the position+vector stack
        uint32_t storeMatrixPos = 30;
        Nitro3Di_SendFifoCommand(GXFifoCommand_StoreMatrix, &storeMatrixPos, 1);
        
        // if texcoord transform mode != 3 (vertex source), set it to 3
        if ((handler->pMaterialRenderData->paramTEXIMAGE_PARAMS & 0xc0000000) != 0xc0000000)
        {
            handler->pMaterialRenderData->paramTEXIMAGE_PARAMS &= ~0xc0000000;
            handler->pMaterialRenderData->paramTEXIMAGE_PARAMS |= 0xc0000000;
            s_command13TexImageParamsFifo[1] = handler->pMaterialRenderData->paramTEXIMAGE_PARAMS;
            Nitro3Di_SendFifoCommand(s_command13TexImageParamsFifo[0],
                &s_command13TexImageParamsFifo[1], 1);
        }

        int callbackStage = (handler->hooks[13] != NULL) ? handler->hookStages[13] : 0;
        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[13](handler);
            callbackStage = (handler->hooks[13] != NULL) ? handler->hookStages[13] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;
        
        if (flagbit6 == 0)
        {
            unsigned int materialWidth = handler->pMaterialRenderData->materialWidth;
            unsigned int materialHeight = handler->pMaterialRenderData->materialHeight;

            s_command13TextureMatrix.entries[0] = materialWidth * 0x8000;
            s_command13TextureMatrix.entries[5] = -materialHeight * 0x8000;
            s_command13TextureMatrix.entries[12] = materialWidth * 0x8000;
            s_command13TextureMatrix.entries[13] = materialHeight * 0x8000;
            Nitro3Di_SendFifoCommand(GXFifoCommand_LoadMat4x4, (uint32_t*)s_command13TextureMatrix.entries, 16);
        }

        if (callbackStage == 2)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[13](handler);
            callbackStage = (handler->hooks[13] != NULL) ? handler->hookStages[13] : 0;
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            unsigned int materialIdx = handler->instructionPointer[1];
            Nitro3DModelMaterials* materialData = handler->modelMaterials;
            Nitro3DMaterial* material = materialData->GetMaterialByIndex(materialIdx);

            if (material->flags & 0x2000)
            {
                intptr_t bit13addr = (intptr_t)(material + 1);
                if (!(material->flags & 2))
                    bit13addr += sizeof(Nitro3DMaterial::ExtensionData_Bit1);
                if (!(material->flags & 4))
                    bit13addr += sizeof(Nitro3DMaterial::ExtensionData_Bit2);
                if (!(material->flags & 8))
                    bit13addr += sizeof(Nitro3DMaterial::ExtensionData_Bit3);

                Nitro3DMaterial::ExtensionData_Bit13* extData = (Nitro3DMaterial::ExtensionData_Bit13*)bit13addr;
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x4, (uint32_t*)extData->matrix.entries, 16);
            }
        }

        if (callbackStage == 3)
        {
            handler->flags &= ~(1 << RCH_FLAG_6);
            handler->hooks[13](handler);
            flagbit6 = handler->flags & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_0))
            {
                Nitro3Di_SendFifoCommand(GXFifoCommand_TranslateMatrix, (uint32_t*)&g_nitro3DRenderConfig.objectRotationPosition.translation, 3);
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&g_nitro3DRenderConfig.objectRotationPosition.rotation.entries, 9);
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)&worldView.entries, 12);
            }
            else if (g_nitro3DRenderConfig.flags & (1 << RENDER_CONFIG_FLAG_1))
            {
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)&worldView.entries, 12);
            }
            else
            {
                const Matrix4x3* invView = Nitro3DRenderConfig::GetInverseViewMatrix();
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)invView, 12);
                Nitro3Di_SendFifoCommand(GXFifoCommand_MultiplyMat4x3, (uint32_t*)&worldView.entries, 12);
            }

            Nitro3D_FlushFifoQueue();
            GXFIFO_MATRIX_MODE = 0; // projection
            GXFIFO_MATRIX_PUSH = 0;
            GXFIFO_MATRIX_IDENTITY = 0;
            Matrix4x4 worldViewAgain;
            while (func_020c54fc(&worldViewAgain) != 0) {}
            GXFIFO_MATRIX_POP = 1;
            GXFIFO_MATRIX_MODE = 3; // texture matrix
            Nitro3Di_SendFifoCommand(GXFifoCommand_LoadMat4x4, (uint32_t*)worldViewAgain.entries, 16);
            
            // casting to short then unsigned short is necessary to match assembly
            uint16_t xCoordIntPart = (short)((unsigned int)(worldViewAgain.entries[12] >> 4) >> 8);
            uint16_t yCoordIntPart = (short)((unsigned int)(worldViewAgain.entries[13] >> 4) >> 8);
            uint32_t texCoordsPacked = xCoordIntPart | (yCoordIntPart << 16);
            Nitro3Di_SendFifoCommand(GXFifoCommand_SetTextureCoords, &texCoordsPacked, 1);
        }
        uint32_t posVectorMode = 2;
        Nitro3Di_SendFifoCommand(GXFifoCommand_SetMatrixMode, &posVectorMode, 1);
        uint32_t retrievePos = 30;
        Nitro3Di_SendFifoCommand(GXFifoCommand_GetMatrix, &retrievePos, 1);
    }
    handler->instructionPointer += 3;
}

void Nitro3D_RenderMeshWithMaterial(Nitro3DModel* model, unsigned int materialIdx, unsigned int meshIdx, int bind)
{
    uint32_t commandParams[7];
    MaterialRenderData targetData;
    
    fix32_t upScale = model->upScale;
    if (upScale != 1 << 12)
    {
        uint32_t upScaleParams[3];
        upScaleParams[0] = upScale;
        upScaleParams[1] = upScale;
        upScaleParams[2] = upScale;
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, upScaleParams, 3);
    }

    if (bind && materialIdx < model->numMaterials)
    {
        Nitro3DModelMaterials* materialData;
        if (model != NULL && model->materialsOffset != 0)
            materialData = (Nitro3DModelMaterials*)((intptr_t)model + model->materialsOffset);
        else
            materialData = NULL;

        Nitro3DMaterial* material = materialData->GetMaterialByIndex(materialIdx);

        // if alpha = 0 don't draw
        if ((material->paramPOLYGON_ATTR & 0x1f0000) == 0)
            return;
        
        commandParams[0] = COMBINE_GXFIFO_COMMANDS3(
            GXFifoCommand_DiffuseAmbientReflect,
            GXFifoCommand_SpecularReflectEmit,
            GXFifoCommand_SetPolygonAttr);
        commandParams[1] = material->paramDIF_AMB;
        commandParams[2] = material->paramSPE_EMI;
        commandParams[3] = material->paramPOLYGON_ATTR;
        // conditionally don't specify alpha?
        if (material->flags & 0x20)
            commandParams[3] &= ~0x001f0000;
        // 2a = TEXIMAGE_PARAMS, 2b = PLTT_BASE
        commandParams[4] = COMBINE_GXFIFO_COMMANDS2(
            GXFifoCommand_SetTexImageParams,
            GXFifoCommand_SetTexturePaletteBase);
        commandParams[5] = material->paramTEXIMAGE_PARAMS;
        commandParams[6] = material->texturePaletteVRAMOffset;
        Nitro3Di_SendFifoCommand(commandParams[0], &commandParams[1], 6);
        
        if (material->flags & 1)
        {
            targetData.flags = 8;
            targetData.materialWidth = material->width;
            targetData.materialHeight = material->height;
            targetData.materialScaleX = material->xScale;
            targetData.materialScaleY = material->yScale;
            intptr_t extraDataAddr = (intptr_t)(material + 1);
            if (material->flags & 2)
                targetData.flags |= 1;
            else
            {
                Nitro3DMaterial::ExtensionData_Bit1* ext = (Nitro3DMaterial::ExtensionData_Bit1*)extraDataAddr;
                targetData.extensionScaleX = ext->scaleX;
                targetData.extensionScaleY = ext->scaleY;
                extraDataAddr += sizeof(*ext);
            }

            if (!(material->flags & 4))
            {
                Nitro3DMaterial::ExtensionData_Bit2* ext = (Nitro3DMaterial::ExtensionData_Bit2*)extraDataAddr;
                targetData.rotationSine = ext->sine;
                targetData.rotationCosine = ext->cosine;
                extraDataAddr += sizeof(*ext);
            }
            else
                targetData.flags |= 2;

            if (!(material->flags & 8))
            {
                Nitro3DMaterial::ExtensionData_Bit3* ext = (Nitro3DMaterial::ExtensionData_Bit3*)extraDataAddr;
                targetData.translateX = ext->translateX;
                targetData.translateY = ext->translateY;
            }
            else
                targetData.flags |= 4;

            if (g_nitro3DLoadMaterialTextureMatrixProcs[model->materialCallbackType])
                g_nitro3DLoadMaterialTextureMatrixProcs[model->materialCallbackType](&targetData);
        }
    }

    if (meshIdx < model->numMeshes)
    {
        Nitro3DNameList* meshList = model->GetMeshList();
        Nitro3DMesh* mesh = meshList->GetEntryFromu32Offset<Nitro3DMesh>(meshIdx);

        Nitro3Di_SendRawDataToFifo(mesh->GetGPUCommands(), mesh->gpuCommandsLength);
    }

    fix32_t downScale = model->downScale;
    if (downScale != 1 << 12)
    {
        uint32_t downScaleParams[3];
        downScaleParams[0] = downScale;
        downScaleParams[1] = downScale;
        downScaleParams[2] = downScale;
        Nitro3Di_SendFifoCommand(GXFifoCommand_ScaleMatrix, downScaleParams, 3);
    }
}