#include "NitroSDK/NSBXX/RenderCommands_Common.h"

static inline int TestBitfieldBit(const unsigned int* bitfield, unsigned int idx)
{
    return bitfield[idx >> 5] & (1 << (idx & 31));
}

static inline int SetBitfieldBit(unsigned int* bitfield, unsigned int idx)
{
    bitfield[idx >> 5] |= (1 << (idx & 31));
}

void ExecuteRenderCommands(RenderCommandHandler* handler)
{
    do
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        uint8_t opcode = handler->instructionPointer_[0];
        data_020f1e08[opcode & 0x1f](handler, opcode & 0xe0);
    } while (!(handler->flags_ & (1 << RCH_FLAG_5)));
}

void SetUpRenderCommandHandler(RenderCommandHandler* handler, ModelRenderContext* modelData)
{
    func_020ca458(0, handler, sizeof(RenderCommandHandler));
    handler->boneMatrixBitfield_[0] = 1;
    handler->flags_ = 1;

    uint8_t* commands = modelData->renderCommandList_;
    if (commands == NULL)
        commands = (uint8_t*)modelData->internalModel_ + modelData->internalModel_->renderCommandsOffset_;
    *(uint8_t* volatile*)&handler->instructionPointer_ = commands;
    
    handler->modelContext_ = modelData;

    if (modelData->internalModel_ != NULL)
        handler->boneList_ = &modelData->internalModel_->boneList_;
    else
        handler->boneList_ = NULL;

    handler->modelMaterials_ = modelData->internalModel_->GetMaterialData();
    handler->meshList_ = modelData->internalModel_->GetMeshList();
    handler->boneMatrixRenderDataScalePopulateProc_ = data_020f1cec[modelData->internalModel_->boneScalingMode_];
    handler->boneMatrixRenderDataSubmitProc_ = data_020f1ce0[modelData->internalModel_->boneScalingMode_];
    handler->textureMatrixCreateProc_ = data_020f1cf8[modelData->internalModel_->materialCallbackType_];

    handler->upScale_ = modelData->internalModel_->upScale_;
    handler->downScale_ = modelData->internalModel_->downScale_;

    if (modelData->renderCommandHook_ != NULL && modelData->renderCommandHookCommandID_ < 32)
    {
        handler->hooks_[modelData->renderCommandHookCommandID_] =
            (void(*)(RenderCommandHandler*))modelData->renderCommandHook_;
        handler->hookStages_[modelData->renderCommandHookCommandID_] = modelData->renderCommandHookStage_;
    }

    if (modelData->flags_ & 1)
        handler->flags_ |= (1 << RCH_FLAG_7);
    if (modelData->flags_ & 2)
        handler->flags_ |= (1 << RCH_FLAG_8);
    if (modelData->flags_ & 4)
        handler->flags_ |= (1 << RCH_FLAG_9);
    if (modelData->flags_ & 8)
        handler->flags_ |= (1 << RCH_FLAG_10);

    if (modelData->preRenderCallback_ != NULL)
    {
        void (*proc)(RenderCommandHandler*) = (void(*)(RenderCommandHandler*))modelData->preRenderCallback_;
        proc(handler);
    }

    ExecuteRenderCommands(handler);
    modelData->flags_ &= ~1;
}

void PopulateBitfieldFromAnimData(unsigned int* bitfield, AnimationData* anim)
{
    while (anim != NULL)
    {
        for (int counter = 0; counter < anim->numEntries_; counter++)
        {
            if (anim->entries_[counter] & 0x100)
                bitfield[counter >> 5] |= 1 << (counter & 0x1f);
        }
        anim = anim->pNext_;
    }
}

void RenderModelFromRenderData(ModelRenderContext* renderData)
{
    RenderCommandHandler handler;
    
    if ((renderData->flags_ & 0x10) == 0x10)
    {
        func_020ca3ec(0, renderData->animatedMaterials_, sizeof(renderData->animatedMaterials_));
        func_020ca3ec(0, renderData->animatedBoneMatrices_, sizeof(renderData->animatedBoneMatrices_));
        func_020ca3ec(0, renderData->animatedVisibilityConditions_, sizeof(renderData->animatedVisibilityConditions_));
        if (renderData->materialAnimations_ != NULL)
            PopulateBitfieldFromAnimData(renderData->animatedMaterials_, renderData->materialAnimations_);
        if (renderData->jointAnimations_ != NULL)
            PopulateBitfieldFromAnimData(renderData->animatedBoneMatrices_, renderData->jointAnimations_);
        if (renderData->visibilityAnimations_ != NULL)
            PopulateBitfieldFromAnimData(renderData->animatedVisibilityConditions_, renderData->visibilityAnimations_);

        renderData->flags_ &= ~0x10;
    }

    if (data_0210a274 != NULL)
    {
        SetUpRenderCommandHandler(data_0210a274, renderData);
    }
    else
    {
        data_0210a274 = &handler;
        SetUpRenderCommandHandler(&handler, renderData);
        data_0210a274 = NULL;
    }
}

// nsbmd docs call this 'nop' though the callback still occurs
void RenderCommand_0(RenderCommandHandler* handler, int modifier)
{
    if (handler->hooks_[0] != NULL)
        handler->hooks_[0](handler);
    handler->instructionPointer_++;
}

// signal end of render commands
void RenderCommand_1(RenderCommandHandler* handler, int modifier)
{
    if (handler->hooks_[1] != NULL)
        handler->hooks_[1](handler);
    handler->flags_ |= (1 << RCH_FLAG_5);
}

// apply visibility condition to be used by animation
// params: condition index, default value (lowest bit used)
void RenderCommand_2(RenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags_ & (1 << RCH_FLAG_9)))
    {
        unsigned int conditionIdx = handler->instructionPointer_[1];
        handler->command2Arg1_ = conditionIdx;
        handler->flags_ |= (1 << RCH_FLAG_2);
        handler->pCommand2Word_ = &handler->scratchCommand2Word_;

        int callbackStage = (handler->hooks_[2] != NULL) ? handler->hookStages_[2] : 0;

        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[2](handler);
            callbackStage = (handler->hooks_[2] != NULL) ? handler->hookStages_[2] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);   
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            ModelRenderContext* modelData = handler->modelContext_;
            if (modelData->visibilityAnimations_ == NULL ||
                !TestBitfieldBit(modelData->animatedVisibilityConditions_, conditionIdx) ||
                !modelData->pfnProcessVisibilityAnimations_(handler->pCommand2Word_, modelData->visibilityAnimations_, conditionIdx))
            {
                *handler->pCommand2Word_ = handler->instructionPointer_[2] & 1;
            }
        }

        if (callbackStage == 2)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[2](handler);
            callbackStage = (handler->hooks_[2] != NULL) ? handler->hookStages_[2] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            if (*handler->pCommand2Word_ != 0)
                handler->flags_ |= (1 << RCH_FLAG_0);
            else
                handler->flags_ &= ~(1 << RCH_FLAG_0);
        }

        if (callbackStage == 3)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[2](handler);
        }
    }
    handler->instructionPointer_ += 3;
}

// load matrix from stack
// parameters: index to load from
void RenderCommand_3(RenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags_ & (1 << RCH_FLAG_9)) && (handler->flags_ & (1 << RCH_FLAG_0)))
    {
        int callbackStage = (handler->hooks_[3] != NULL) ? handler->hookStages_[3] : 0;

        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[3](handler);
            callbackStage = (handler->hooks_[3] != NULL) ? handler->hookStages_[3] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            uint32_t stackPos = handler->instructionPointer_[1];
            if (!(handler->flags_ & (1 << RCH_FLAG_8)))
                SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &stackPos, 1);
        }

        if (callbackStage == 3)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[3](handler);
        }
    }
    handler->instructionPointer_ += 2;
}

void MaterialBindProc(RenderCommandHandler* handler, int modifier, NSBXXMaterial* material, unsigned int idx)
{
    handler->boundMaterial_ = idx;
    handler->flags_ |= (1 << RCH_FLAG_3);
    handler->pMaterialRenderData_ = &handler->scratchMaterialRenderData_;

    int callbackStage = (handler->hooks_[4] != NULL) ? handler->hookStages_[4] : 0;

    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[4](handler);
        callbackStage = (handler->hooks_[4] != NULL) ? handler->hookStages_[4] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0)
    {
        MaterialRenderData* targetData;
        MaterialRenderData* modelMaterialArray = handler->modelContext_->materialRenderDataArray_;
        if (modelMaterialArray != NULL && !(handler->flags_ & (1 << RCH_FLAG_7)))
            targetData = &modelMaterialArray[idx];
        else if ((modifier == 0x20 || modifier == 0x40) && TestBitfieldBit(handler->materialBitfield_, idx))
        {
            if (modelMaterialArray != NULL)
                targetData = &modelMaterialArray[idx];
            else
                targetData = &data_0210a278[idx];
        }
        else
        {
            if (modelMaterialArray != NULL)
            {
                SetBitfieldBit(handler->materialBitfield_, idx);
                targetData = &handler->modelContext_->materialRenderDataArray_[idx];
            }
            else
            {
                if (!(modifier != 0x40))
                {
                    SetBitfieldBit(handler->materialBitfield_, idx);
                    targetData = &data_0210a278[idx];
                }
                else
                    targetData = &handler->scratchMaterialRenderData_;
            }

            targetData->flags_ = 0;
            NSBXXMaterial* lookupMaterial = handler->modelMaterials_->GetMaterialByIndex(idx);
            if (lookupMaterial->flags_ & 0x20)
                targetData->flags_ |= 0x20;
            
            int difambMask = data_020e9240[(material->flags_ >> 6) & 7];
            targetData->paramDIF_AMB_ = (data_0210a010.diffuseAmbientArg & ~difambMask) |
                (material->paramDIF_AMB_ & difambMask);

            int speemiMask = data_020e9240[(material->flags_ >> 9) & 7];
            targetData->paramSPE_EMI_ = (data_0210a010.specularArg & ~speemiMask) |
                (material->paramSPE_EMI_ & speemiMask);

            targetData->paramPOLYGON_ATTR_ = (data_0210a010.polygonAttrArg & ~material->maskPOLYGON_ATTR_) |
                (material->paramPOLYGON_ATTR_ & material->maskPOLYGON_ATTR_);
            targetData->paramTEXIMAGE_PARAMS_ = material->paramTEXIMAGE_PARAMS_;
            targetData->texturePaletteBase_ = material->texturePaletteVRAMOffset_;

            if (material->flags_ & 1)
            {
                intptr_t extensionDataAddress = (intptr_t)(material + 1);
                if (!(material->flags_ & 2))
                {
                    NSBXXMaterial::ExtensionData_Bit1* extensionData = 
                        (NSBXXMaterial::ExtensionData_Bit1*)extensionDataAddress;
                    targetData->extensionScaleX_ = extensionData->scaleX_;
                    targetData->extensionScaleY_ = extensionData->scaleY_;
                    extensionDataAddress += sizeof(*extensionData);
                }
                else
                    targetData->flags_ |= 1;

                if (!(material->flags_ & 4))
                {
                    NSBXXMaterial::ExtensionData_Bit2* extensionData =
                        (NSBXXMaterial::ExtensionData_Bit2*)extensionDataAddress;
                    targetData->rotationSine_ = extensionData->sine_;
                    targetData->rotationCosine_ = extensionData->cosine_;
                    extensionDataAddress += sizeof(*extensionData);
                }
                else
                    targetData->flags_ |= 2;
                
                if (!(material->flags_ & 8))
                {
                    NSBXXMaterial::ExtensionData_Bit3* extensionData = 
                        (NSBXXMaterial::ExtensionData_Bit3*)extensionDataAddress;
                    targetData->translateX_ = extensionData->translateX_;
                    targetData->translateY_ = extensionData->translateY_;
                }
                else 
                    targetData->flags_ |= 4;

                targetData->flags_ |= 8;
            }

            ModelRenderContext* modelData = handler->modelContext_;
            if (modelData->materialAnimations_ != NULL && TestBitfieldBit(modelData->animatedMaterials_, idx))
                modelData->pfnProcessMaterialAnimations_(targetData, modelData->materialAnimations_, idx);

            if (targetData->flags_ & (0x10 | 0x08))
            {
                targetData->materialWidth_ = material->width_;
                targetData->materialHeight_ = material->height_;
                targetData->materialxScale_ = material->xScale_;
                targetData->materialyScale_ = material->yScale_;
            }
        }      
        handler->pMaterialRenderData_ = targetData;
    }

    if (callbackStage == 2)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[4](handler);
        callbackStage = (handler->hooks_[4] != NULL) ? handler->hookStages_[4] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0)
    {
        MaterialRenderData* targetData = handler->pMaterialRenderData_;
        if ((targetData->paramPOLYGON_ATTR_ & 0x1f0000) != 0) // alpha != 0
        {
            if (targetData->flags_ & 0x20)
                targetData->paramPOLYGON_ATTR_ &= ~0x1f0000;
            handler->flags_ &= ~(1 << RCH_FLAG_1);
            if (!(handler->flags_ & (1 << RCH_FLAG_8)))
            {
                uint32_t commands[7];
                commands[0] = COMBINE_GXFIFO_COMMANDS3(
                    GXFifoCommand_DiffuseAmbientReflect,
                    GXFifoCommand_SpecularReflectEmit,
                    GXFifoCommand_SetPolygonAttr);
                commands[1] = targetData->paramDIF_AMB_;
                commands[2] = targetData->paramSPE_EMI_;
                commands[3] = targetData->paramPOLYGON_ATTR_;
                commands[4] = COMBINE_GXFIFO_COMMANDS2(
                    GXFifoCommand_SetTexImageParams,
                    GXFifoCommand_SetTexturePaletteBase);
                commands[5] = targetData->paramTEXIMAGE_PARAMS_;
                commands[6] = targetData->texturePaletteBase_;
                SubmitCommandToGeometryFifo(commands[0], &commands[1], 6);

                if (targetData->flags_ & (0x10 | 0x08))
                    handler->textureMatrixCreateProc_(targetData);
            }
        }
        else
            handler->flags_ |= (1 << RCH_FLAG_1);
    }

    if (callbackStage == 3)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[4](handler);
    }
}

// bind material
// parameters: index of material within the model's MaterialList
void RenderCommand_4(RenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags_ & (1 << RCH_FLAG_9)))
    {
        int materialIdx = handler->instructionPointer_[1];
        if ((handler->flags_ & (1 << RCH_FLAG_0)) || !(handler->flags_ & (1 << RCH_FLAG_3)) || materialIdx != handler->boundMaterial_)
        {
            NSBXXMaterial* material = handler->modelMaterials_->GetMaterialByIndex(materialIdx);
            // MaterialBindProc
            data_020f1d08.materialBindFunctions[material->unk_0](handler, modifier, material, materialIdx);
        }
    }
    handler->instructionPointer_ += 2;
}

void MeshDrawProc(RenderCommandHandler* handler, int modifier, NSBXXMesh* mesh, unsigned int meshIdx)
{
    int callbackStage = (handler->hooks_[5] != NULL) ? handler->hookStages_[5] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[5](handler);
        callbackStage = (handler->hooks_[5] != NULL) ? handler->hookStages_[5] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0 && !(handler->flags_ & (1 << RCH_FLAG_8)))
        SendRawDataToGeometryFifo(mesh->GetGPUCommands(), mesh->gpuCommandsLength_);

    if (callbackStage == 2)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[5](handler);
        callbackStage = (handler->hooks_[5] != NULL) ? handler->hookStages_[5] : 0;
    }

    if (callbackStage == 3)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[5](handler);
    }
}

// draw mesh
// parameters: index of mesh to draw
void RenderCommand_5(RenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags_ & (1 << RCH_FLAG_9)) && (handler->flags_ & (1 << RCH_FLAG_0)) && !(handler->flags_ & (1 << RCH_FLAG_1)))
    {
        int meshIdx = handler->instructionPointer_[1];
        NSBXXNameList* meshList = handler->meshList_;
        NSBXXMesh* mesh = meshList->GetEntryFromu32Offset_v2<NSBXXMesh>(meshIdx);
        data_020f1d08.meshDrawFunctions[mesh->unk_0](handler, modifier, mesh, meshIdx);
    }
    handler->instructionPointer_ += 2;
}

// multiply current matrix by bone matrix (on the left)
// parameters: 
// 1) index of bone matrix
// 2) index of parent bone's bone matrix (needed for certain scaling operations)
// 3) unknown, seemingly unused
// if modifier & 0x20: stack position to store result at the end
// if modifier & 0x40: stack position of matrix to retrieve at the start, i.e. 
// you'll retrieve a matrix off the stack and pre-multiply that by the bone matrix
void RenderCommand_6(RenderCommandHandler* handler, int modifier)
{
    int boneIdx = handler->instructionPointer_[1];
    int numBytesConsumed = 4; // by default it's opcode + 3 args
    handler->currentBoneMatrix_ = boneIdx;
    handler->flags_ |= (1 << RCH_FLAG_4);
    if (handler->flags_ & (1 << RCH_FLAG_10))
    {
        if (modifier == 0x40 || modifier == 0x60)
            numBytesConsumed++;
        if (modifier == 0x20 || modifier == 0x60)
        {
            numBytesConsumed++;
            if (!(handler->flags_ & (1 << RCH_FLAG_8)))
            {
                uint32_t retrievalPosition = handler->instructionPointer_[4];
                SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
            }
        }
        handler->instructionPointer_ += numBytesConsumed;
        return;
    }

    // if the 0x40 bit is set...
    if (modifier == 0x40 || modifier == 0x60)
    {
        // the presence of the 0x20 bit means we have to look one further
        uint8_t* instructionPtr = handler->instructionPointer_;
        uint32_t retrievalPosition;
        if (modifier == 0x40) 
            retrievalPosition = instructionPtr[4];
        else 
            retrievalPosition = instructionPtr[5];
        numBytesConsumed++;
        if (!(handler->flags_ & (1 << RCH_FLAG_8)))
            SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
    }

    handler->pBoneMatrixRenderData_ = &handler->scratchBoneMatrixRenderData_;
    int callbackStage = (handler->hooks_[6] != NULL) ? handler->hookStages_[6] : 0;

    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[6](handler);
        callbackStage = (handler->hooks_[6] != NULL) ? handler->hookStages_[6] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (flagbit6 == 0)
    {
        BoneMatrixRenderData* targetData;
        BoneMatrixRenderData* modelBoneArray = handler->modelContext_->boneMatrixRenderDataArray_;

        bool bit7clear;
        if (modelBoneArray != NULL)
        {
            targetData = &modelBoneArray[boneIdx];
            bit7clear = !(handler->flags_ & (1 << RCH_FLAG_7));
        }
        else
        {
            targetData = &handler->scratchBoneMatrixRenderData_;
            bit7clear = false;
        }

        if (!bit7clear)
        {
            targetData->flags_ = 0;
            if (handler->modelContext_->jointAnimations_ == NULL ||
                !handler->modelContext_->pfnProcessJointAnimations_(targetData, handler->modelContext_->jointAnimations_, boneIdx))
            {
                NSBXXBoneMatrix* boneMatrix = handler->boneList_->GetEntryFromu32Offset_v2<NSBXXBoneMatrix>(boneIdx);
                intptr_t boneMatrixExtraPtr = (intptr_t)(boneMatrix + 1);
                if ((boneMatrix->flags_ & 1)) // has no translation component
                    targetData->flags_ |= 4;
                else
                {
                    NSBXXBoneMatrix::Translation* translation = 
                        (NSBXXBoneMatrix::Translation*)boneMatrixExtraPtr;
                    targetData->translate_.x = translation->x;
                    targetData->translate_.y = translation->y;
                    targetData->translate_.z = translation->z;
                    boneMatrixExtraPtr += sizeof(*translation);
                }

                if (boneMatrix->flags_ & 2) // does not have any kind of rotation
                    targetData->flags_ |= 2;
                else if (boneMatrix->flags_ & 8) // use pivot matrix
                {
                    int form = (boneMatrix->flags_ & 0xf0) >> 4;
                    NSBXXBoneMatrix::PivotMatrixData* pivot = 
                        (NSBXXBoneMatrix::PivotMatrixData*)boneMatrixExtraPtr;
                    fix32_t entryA = pivot->a;
                    fix32_t entryB = pivot->b;
                    func_020ca7d0(&targetData->rotationMatrix_);
                    
                    fix32_t unit;
                    if (boneMatrix->flags_ & 0x100)
                        unit = 0xfffff000; // -1.0
                    else
                        unit = 0x1000; // +1.0

                    targetData->rotationMatrix_.entries[form] = unit;

                    uint8_t indexA = data_020e9260[form].a;
                    uint8_t indexB = data_020e9260[form].b;

                    targetData->rotationMatrix_.entries[indexA] = entryA;
                    targetData->rotationMatrix_.entries[indexB] = entryB;

                    fix32_t entryC = (boneMatrix->flags_ & 0x200) ? -entryB : entryB;
                    uint8_t indexC = data_020e9260[form].c;
                    targetData->rotationMatrix_.entries[indexC] = entryC;

                    fix32_t entryD = (boneMatrix->flags_ & 0x400) ? -entryA : entryA;
                    uint8_t indexD = data_020e9260[form].d;
                    targetData->rotationMatrix_.entries[indexD] = entryD;
                    boneMatrixExtraPtr += sizeof(*pivot);
                }
                else // use specified matrix
                {
                    // casting from fix16_t to fix32_t, this is okay because
                    // both have 12 bits after the point
                    targetData->rotationMatrix_.entries[0] = boneMatrix->m_11;
                    NSBXXBoneMatrix::RotationMatrixData* rotation = 
                        (NSBXXBoneMatrix::RotationMatrixData*)boneMatrixExtraPtr;
                    targetData->rotationMatrix_.entries[1] = rotation->entries[0];
                    targetData->rotationMatrix_.entries[2] = rotation->entries[1];
                    targetData->rotationMatrix_.entries[3] = rotation->entries[2];
                    targetData->rotationMatrix_.entries[4] = rotation->entries[3];
                    targetData->rotationMatrix_.entries[5] = rotation->entries[4];
                    targetData->rotationMatrix_.entries[6] = rotation->entries[5];
                    targetData->rotationMatrix_.entries[7] = rotation->entries[6];
                    targetData->rotationMatrix_.entries[8] = rotation->entries[7];
                    boneMatrixExtraPtr += sizeof(*rotation);
                }

                handler->boneMatrixRenderDataScalePopulateProc_(targetData,
                    (NSBXXBoneMatrix::Scaling*)boneMatrixExtraPtr,
                    handler->instructionPointer_, boneMatrix->flags_);
            }
        }
        handler->pBoneMatrixRenderData_ = targetData;
    }

    if (callbackStage == 2)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[6](handler);
        callbackStage = (handler->hooks_[6] != NULL) ? handler->hookStages_[6] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;
    
    if (flagbit6 == 0 && !(handler->flags_ & (1 << RCH_FLAG_8)))
    {
        handler->boneMatrixRenderDataSubmitProc_(handler->pBoneMatrixRenderData_);
    }

    handler->pBoneMatrixRenderData_ = NULL;
    if (callbackStage == 3)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[6](handler);
        callbackStage = (handler->hooks_[6] != NULL) ? handler->hookStages_[6] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (modifier == 0x20 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (flagbit6 == 0 && !(handler->flags_ & (1 << RCH_FLAG_8)))
        {
            uint32_t storagePosition = handler->instructionPointer_[4];
            SubmitCommandToGeometryFifo(GXFifoCommand_StoreMatrix, &storagePosition, 1);
        }
    }

    handler->instructionPointer_ += numBytesConsumed;
}

// seems to be used for free billboarding, i.e. making stuff turn in any
// direction to face the camera. Noticeable with evac animation.
// parameters:
// 1) ???
// if modifier & 0x20: stack position to store result at the end
// if modifier & 0x40: stack position of matrix to retrieve at the start, i.e. 
// you'll retrieve a matrix off the stack and pre-multiply that by the bone matrix
void RenderCommand_7(RenderCommandHandler* handler, int modifier)
{
    int numBytesConsumed = 2;
    Vector3fix* pTranslation = &data_020f1d78.rotationTranslation.translation;
    Vector3fix* pScaling = &data_020f1d78.scaling;
    if (handler->flags_ & (1 << RCH_FLAG_9))
    {
        if (modifier == 0x40 || modifier == 0x60)
            numBytesConsumed++;
        if (modifier == 0x20 || modifier == 0x60)
            numBytesConsumed++;
        handler->instructionPointer_ += numBytesConsumed;
        return;
    }

    if (modifier == 0x40 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (!(handler->flags_ & (1 << RCH_FLAG_8)))
        {
            uint8_t* instructionPtr = handler->instructionPointer_;
            uint32_t retrievalPosition;
            if (modifier == 0x40)
                retrievalPosition = instructionPtr[2];
            else
                retrievalPosition = instructionPtr[3];
            SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
        }
    }

    int callbackStage = (handler->hooks_[7] != NULL) ? handler->hookStages_[7] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[7](handler);
        callbackStage = (handler->hooks_[7] != NULL) ? handler->hookStages_[7] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (!(handler->flags_ & (1 << RCH_FLAG_8)) && flagbit6 == 0)
    {
        SendQueuedDataToGeometryFifo();
        GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_PushMatrix, GXFifoCommand_LoadMatIdentity);
        GXFIFO = 0; // matrix mode 0: projection
        GXFIFO = 0; // why is this here?
        Matrix4x4 clipMatrix;
        // get the current world * view matrix (we set projection matrix to identity, so
        // it doesn't contribute here)
        while (func_020c54fc(&clipMatrix) != 0) {}
        if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            const Matrix4x3* worldView4x3 = RenderConfig::GetCombinedWorldViewMatrix();
            Matrix4x4 worldView;
            Mat4x3_ConvertTo4x4(worldView4x3, &worldView);
            Mat4x4_Multiply(&clipMatrix, &worldView, &clipMatrix);
        }
        else if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            Matrix4x4 view4x4;
            Mat4x3_ConvertTo4x4(&data_0210a010.viewMatrix, &view4x4);
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

        if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&data_020f1d78.popMatrixParameter, &GXFIFO, 8); // pass pop / matrix mode commands
            // load the matrix returned by func_020b3a24(), inverse of func_020b39ec()
            func_020ca430(RenderConfig::GetInverseCombinedWorldViewMatrix(), &GXFIFO, 4*3 * sizeof(fix32_t));
            // the matrix to multiply by is a translation matrix, so we're composing
            // scaling with translation (remember the final operation represents
            // the first matrix transformation to carry out)
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&data_020f1d78.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&data_020f1d78.popMatrixParameter, &GXFIFO, 8);
            func_020ca430(RenderConfig::GetInverseViewMatrix(), &GXFIFO, 4*3 * sizeof(fix32_t));
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&data_020f1d78.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else
        {
            // send all the commands at once: pop the matrix, set matrix mode to 2,
            // then load a translation + scaling matrix
            func_020ca430(&data_020f1d78, &GXFIFO, sizeof(data_020f1d78));
        }

    }

    int newFlagbit6;
    if (callbackStage == 3)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[7](handler);
        newFlagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        newFlagbit6 = 0;

    if (modifier == 0x20 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (newFlagbit6 == 0 && !(handler->flags_ & (1 << RCH_FLAG_8)))
        {
            uint32_t storePosition = handler->instructionPointer_[2];
            SubmitCommandToGeometryFifo(GXFifoCommand_StoreMatrix, &storePosition, 1);
        }
    }
    handler->instructionPointer_ += numBytesConsumed;
}

// seems to be used for axis-aligned billboarding - noticeable on trees in
// Angel Falls & lights in cave grottos
void RenderCommand_8(RenderCommandHandler* handler, int modifier)
{
    int numBytesConsumed = 2;
    Vector3fix* pTranslation = &data_020f1dc0.rotationTranslation.translation;
    Vector3fix* pScaling = &data_020f1dc0.scaling;
    Matrix3x3* pRotation = &data_020f1dc0.rotationTranslation.rotation;
    if (handler->flags_ & (1 << RCH_FLAG_9))
    {
        if (modifier == 0x40 || modifier == 0x60)
            numBytesConsumed++;
        if (modifier == 0x20 || modifier == 0x60)
            numBytesConsumed++;
        handler->instructionPointer_ += numBytesConsumed;
        return;
    }

    if (modifier == 0x40 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (!(handler->flags_ & (1 << RCH_FLAG_8)))
        {
            uint8_t* instructionPtr = handler->instructionPointer_;
            uint32_t retrievalPosition;
            if (modifier == 0x40)
                retrievalPosition = instructionPtr[2];
            else
                retrievalPosition = instructionPtr[3];
            SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &retrievalPosition, 1);
        }
    }

    int callbackStage = (handler->hooks_[8] != NULL) ? handler->hookStages_[8] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[8](handler);
        callbackStage = (handler->hooks_[8] != NULL) ? handler->hookStages_[8] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (!(handler->flags_ & (1 << RCH_FLAG_8)) && flagbit6 == 0)
    {
        SendQueuedDataToGeometryFifo();
        GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_SetMatrixMode, GXFifoCommand_PushMatrix, GXFifoCommand_LoadMatIdentity);
        GXFIFO = 0; // matrix mode 0: projection
        GXFIFO = 0; // why is this here?
        Matrix4x4 clipMatrix;
        while (func_020c54fc(&clipMatrix) != 0) {}
        if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            const Matrix4x3* worldView4x3 = RenderConfig::GetCombinedWorldViewMatrix();
            Matrix4x4 worldView;
            Mat4x3_ConvertTo4x4(worldView4x3, &worldView);
            Mat4x4_Multiply(&clipMatrix, &worldView, &clipMatrix);
        }
        else if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            Matrix4x4 view4x4;
            Mat4x3_ConvertTo4x4(&data_0210a010.viewMatrix, &view4x4);
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

        if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_0))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&data_020f1dc0.popMatrixParameter, &GXFIFO, 8); // pass pop / matrix mode commands
            func_020ca430(RenderConfig::GetInverseCombinedWorldViewMatrix(), &GXFIFO, 3 * 4 * sizeof(fix32_t));
            // the 4x3 matrix is a rotate-then-translate matrix. Recall that matrix multiplication
            // commands put the parameter on the inside, so we are actually ending 
            // up with (scale) -> (rotate then translate) -> (apply matrix from above)
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&data_020f1dc0.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_1))
        {
            GXFIFO = COMBINE_GXFIFO_COMMANDS3(GXFifoCommand_PopMatrix, GXFifoCommand_SetMatrixMode, GXFifoCommand_LoadMat4x3);
            func_020ca430(&data_020f1dc0.popMatrixParameter, &GXFIFO, 8);
            func_020ca430(RenderConfig::GetInverseViewMatrix(), &GXFIFO, 4*3 * sizeof(fix32_t));
            // the 4x3 matrix is a rotate-then-translate matrix. Recall that matrix multiplication
            // commands put the parameter on the inside, so we are actually ending 
            // up with (scale) -> (rotate then translate) -> (apply matrix from above)
            GXFIFO = COMBINE_GXFIFO_COMMANDS2(GXFifoCommand_MultiplyMat4x3, GXFifoCommand_ScaleMatrix);
            func_020ca430(&data_020f1dc0.rotationTranslation, &GXFIFO, (4*3 + 3) * sizeof(fix32_t));
        }
        else
        {
            // send all the commands at once: pop the matrix, set matrix mode to 2,
            // then pre-apply a translation, then pre-apply a scaling
            func_020ca430(&data_020f1dc0, &GXFIFO, sizeof(data_020f1dc0));
        }
    }

    int newFlagbit6;
    if (callbackStage == 3)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[8](handler);
        newFlagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        newFlagbit6 = 0;

    if (modifier == 0x20 || modifier == 0x60)
    {
        numBytesConsumed++;
        if (newFlagbit6 == 0 && !(handler->flags_ & (1 << RCH_FLAG_8)))
        {
            uint32_t storePosition = handler->instructionPointer_[2];
            SubmitCommandToGeometryFifo(GXFifoCommand_StoreMatrix, &storePosition, 1);
        }
    }
    handler->instructionPointer_ += numBytesConsumed;
}

void RenderCommand_9(RenderCommandHandler* handler, int modifier)
{
    int64_t weight = 0;
    NSBXXInternalModel* model = handler->modelContext_->internalModel_;    
    const NSBXXInvBindMatrix* invBindMatrices = (NSBXXInvBindMatrix*)((intptr_t)model + model->inverseBindsOffset_);
    unsigned int numTerms = handler->instructionPointer_[2];
    uint8_t* instructionPtr = handler->instructionPointer_ + 3;
    
    NSBXXInvBindMatrix localCopy;
    func_020ca458(0, &localCopy, sizeof(NSBXXInvBindMatrix));
    SendQueuedDataToGeometryFifo();

    GXFIFO_MATRIX_MODE = 0; // projection
    GXFIFO_MATRIX_STORE = 1;
    GXFIFO_MATRIX_IDENTITY = 0;
    GXFIFO_MATRIX_MODE = 2; // position+vector

    Matrix4x4* cachedPosMatrix;
    Matrix3x3* cachedDirMatrix;
    
    for (unsigned int termCounter = 0; termCounter < numTerms; termCounter++)
    {
        unsigned int inverseBindIndex = instructionPtr[1];
        
        int inBitfield = TestBitfieldBit(handler->invBindBitfield_, inverseBindIndex);
        cachedPosMatrix = &data_0210b078.matrixData[inverseBindIndex].mat4x4;
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
        cachedDirMatrix = &data_0210b078.matrixData[inverseBindIndex].mat3x3;
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
    GXFIFO_MATRIX_STORE = handler->instructionPointer_[1];
    handler->instructionPointer_ += 3 + handler->instructionPointer_[2] * 3;
}

// run raw gxfifo commands at the given offset and length.
// notably, the instruction pointer only advances past the offset and
// length data, so presumably this is stored past the end, like constants
// in assembly coming after the return statement
void RenderCommand_10(RenderCommandHandler* handler, int modifier)
{
    int callbackStage = (handler->hooks_[10] != NULL) ? handler->hookStages_[10] : 0;
    unsigned int flagbit6;
    if (callbackStage == 1)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[10](handler);
        callbackStage = (handler->hooks_[10] != NULL) ? handler->hookStages_[10] : 0;
        flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
    }
    else
        flagbit6 = 0;

    if (!(handler->flags_ & (1 << RCH_FLAG_8)) && flagbit6 == 0)
    {
        uint8_t* instructionPtr = handler->instructionPointer_;
        uint32_t startOffset = (instructionPtr[1] | (instructionPtr[2] << 8) | (instructionPtr[3] << 16) | (instructionPtr[4] << 24));
        uint32_t length = (instructionPtr[5] | (instructionPtr[6] << 8) | (instructionPtr[7] << 16) | (instructionPtr[8] << 24));
        SendRawDataToGeometryFifo((uint32_t*)(instructionPtr + startOffset), length); 
    }

    if (callbackStage == 3)
    {
        handler->flags_ &= ~(1 << RCH_FLAG_6);
        handler->hooks_[10](handler);
    }

    handler->instructionPointer_ += 9;
}

void RenderCommand_11(RenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags_ & (1 << RCH_FLAG_8)) && !(handler->flags_ & (1 << RCH_FLAG_9)))
    {
        uint32_t scaleParams[3];
        if (modifier == 0)
            scaleParams[0] = scaleParams[1] = scaleParams[2] = handler->upScale_;
        else
            scaleParams[0] = scaleParams[1] = scaleParams[2] = handler->downScale_;

        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, scaleParams, 3);
    }
    handler->instructionPointer_++;
}

// This instruction isn't explained in the nsbmd docs. Given the use of normals
// as inputs for texture coordinates and the same camera matrices showing up
// as in commands 7 and 8, this might be something like matcap lighting
// 2 parameters: 1) index of material in model's material list to use
// second parameter is unknown / seems to be unused
void RenderCommand_12(RenderCommandHandler* handler, int modifier)
{
    Matrix3x3 directionMat3x3;
    if (!(handler->flags_ & (1 << RCH_FLAG_9)) && (handler->flags_ & (1 << RCH_FLAG_0)))
    {
        // if texcoord transformation mode != 2 (normal), set it to 2 and submit
        // the new params. In this mode, final texture coords to be used
        // are given by (normal) * TextureMatrix + (raw coords)
        // with raw coords being the argument to GXFifoCommand_SetTextureCoords
        if ((handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_ & 0xc0000000) != 0x80000000)
        {
            handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_ &= ~0xc0000000;
            handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_ |= 0x80000000;
            data_020f1d08.texImageParamsArg_c_ = handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_;
            SubmitCommandToGeometryFifo(data_020f1d08.commandTexImageParams_8_,
                &data_020f1d08.texImageParamsArg_c_, 1);
        }
        // switch the matrix mode to texture
        uint32_t textureMatrixMode = 3;
        SubmitCommandToGeometryFifo(GXFifoCommand_SetMatrixMode, &textureMatrixMode, 1);

        int callbackStage = (handler->hooks_[12] != NULL) ? handler->hookStages_[12] : 0;
        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[12](handler);
            callbackStage = (handler->hooks_[12] != NULL) ? handler->hookStages_[12] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;
            
        if (flagbit6 == 0)
        {
            unsigned int materialWidth = handler->pMaterialRenderData_->materialWidth_;
            unsigned int materialHeight = handler->pMaterialRenderData_->materialHeight_;

            uint32_t scaleParams[3];
            scaleParams[0] = materialWidth * 0x8000;
            scaleParams[1] = -materialHeight * 0x8000;
            scaleParams[2] = 0x10000;
            SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, scaleParams, 3);
            // Due to 4 bits after the fixed point, this * 8 is really a division by 2
            short xcoord = materialWidth * 8;
            short ycoord = materialHeight * 8;
            uint32_t texCoordPack = (unsigned short)xcoord | ((unsigned short)ycoord << 16);
            SubmitCommandToGeometryFifo(GXFifoCommand_SetTextureCoords, &texCoordPack, 1);
        }

        if (callbackStage == 2)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[12](handler);
            callbackStage = (handler->hooks_[12] != NULL) ? handler->hookStages_[12] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            unsigned int materialIdx = handler->instructionPointer_[1];
            NSBXXMaterial* material = handler->modelMaterials_->GetMaterialByIndex(materialIdx);
            if (material->flags_ & 0x2000)
            {
                intptr_t bit13data = (intptr_t)(material + 1);
                if (!(material->flags_ & 2))
                    bit13data += sizeof(NSBXXMaterial::ExtensionData_Bit1);
                if (!(material->flags_ & 4))
                    bit13data += sizeof(NSBXXMaterial::ExtensionData_Bit2);
                if (!(material->flags_ & 8))
                    bit13data += sizeof(NSBXXMaterial::ExtensionData_Bit3);
                
                NSBXXMaterial::ExtensionData_Bit13* ptr = (NSBXXMaterial::ExtensionData_Bit13*)bit13data;
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x4, (uint32_t*)ptr->matrix_.entries, 16);
            }
        }

        if (callbackStage == 3)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[12](handler);
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            uint32_t posVectorMode = 2;
            SubmitCommandToGeometryFifo(GXFifoCommand_SetMatrixMode, &posVectorMode, 1);
            
            GetCurrentPositionAndDirectionMatrices(NULL, &directionMat3x3);
            uint32_t textureMode = 3;
            SubmitCommandToGeometryFifo(GXFifoCommand_SetMatrixMode, &textureMode, 1);
            if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_0))
            {
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&data_0210a010.viewMatrix.entries, 9);
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&data_0210a010.objectRotationPosition.rotation.entries, 9);
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&directionMat3x3.entries, 9);
            }
            else if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_1))
            {
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&data_0210a010.viewMatrix.entries, 9);
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&directionMat3x3.entries, 9);
            }
            else
            {
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&directionMat3x3.entries, 9);
            }
        }
        // switch back to position+vector mode
        uint32_t endMode = 2;
        SubmitCommandToGeometryFifo(GXFifoCommand_SetMatrixMode, &endMode, 1);
    }
    handler->instructionPointer_ += 3;
}

// I have no idea what this is, but it's texture related. I couldn't find any
// use of it in the game.
// 2 parameters: 1) index of material in model's material list, 2) unknown/unused
void RenderCommand_13(RenderCommandHandler* handler, int modifier)
{
    if (!(handler->flags_ & (1 << RCH_FLAG_9)) && (handler->flags_ & (1 << RCH_FLAG_0)))
    {
        Matrix4x3 worldView;
        GetCurrentPositionAndDirectionMatrices(&worldView, NULL);

        // store onto the position+vector stack
        uint32_t storeMatrixPos = 30;
        SubmitCommandToGeometryFifo(GXFifoCommand_StoreMatrix, &storeMatrixPos, 1);
        
        // if texcoord transform mode != 3 (vertex source), set it to 3
        if ((handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_ & 0xc0000000) != 0xc0000000)
        {
            handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_ &= ~0xc0000000;
            handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_ |= 0xc0000000;
            data_020f1d08.texImageParamsArg_4_ = handler->pMaterialRenderData_->paramTEXIMAGE_PARAMS_;
            SubmitCommandToGeometryFifo(data_020f1d08.commandTexImageParams_0_,
                &data_020f1d08.texImageParamsArg_4_, 1);
        }

        int callbackStage = (handler->hooks_[13] != NULL) ? handler->hookStages_[13] : 0;
        unsigned int flagbit6;
        if (callbackStage == 1)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[13](handler);
            callbackStage = (handler->hooks_[13] != NULL) ? handler->hookStages_[13] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;
        
        if (flagbit6 == 0)
        {
            unsigned int materialWidth = handler->pMaterialRenderData_->materialWidth_;
            unsigned int materialHeight = handler->pMaterialRenderData_->materialHeight_;

            data_020f1d08.command13Matrix4x4_.entries[0] = materialWidth * 0x8000;
            data_020f1d08.command13Matrix4x4_.entries[5] = -materialHeight * 0x8000;
            data_020f1d08.command13Matrix4x4_.entries[12] = materialWidth * 0x8000;
            data_020f1d08.command13Matrix4x4_.entries[13] = materialHeight * 0x8000;
            SubmitCommandToGeometryFifo(GXFifoCommand_LoadMat4x4, (uint32_t*)data_020f1d08.command13Matrix4x4_.entries, 16);
        }

        if (callbackStage == 2)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[13](handler);
            callbackStage = (handler->hooks_[13] != NULL) ? handler->hookStages_[13] : 0;
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            unsigned int materialIdx = handler->instructionPointer_[1];
            NSBXXModelMaterialData* materialData = handler->modelMaterials_;
            NSBXXMaterial* material = materialData->GetMaterialByIndex(materialIdx);

            if (material->flags_ & 0x2000)
            {
                intptr_t bit13addr = (intptr_t)(material + 1);
                if (!(material->flags_ & 2))
                    bit13addr += sizeof(NSBXXMaterial::ExtensionData_Bit1);
                if (!(material->flags_ & 4))
                    bit13addr += sizeof(NSBXXMaterial::ExtensionData_Bit2);
                if (!(material->flags_ & 8))
                    bit13addr += sizeof(NSBXXMaterial::ExtensionData_Bit3);

                NSBXXMaterial::ExtensionData_Bit13* extData = (NSBXXMaterial::ExtensionData_Bit13*)bit13addr;
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x4, (uint32_t*)extData->matrix_.entries, 16);
            }
        }

        if (callbackStage == 3)
        {
            handler->flags_ &= ~(1 << RCH_FLAG_6);
            handler->hooks_[13](handler);
            flagbit6 = handler->flags_ & (1 << RCH_FLAG_6);
        }
        else
            flagbit6 = 0;

        if (flagbit6 == 0)
        {
            if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_0))
            {
                SubmitCommandToGeometryFifo(GXFifoCommand_TranslateMatrix, (uint32_t*)&data_0210a010.objectRotationPosition.translation, 3);
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat3x3, (uint32_t*)&data_0210a010.objectRotationPosition.rotation.entries, 9);
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)&worldView.entries, 12);
            }
            else if (data_0210a010.flags & (1 << RENDER_CONFIG_FLAG_1))
            {
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)&worldView.entries, 12);
            }
            else
            {
                const Matrix4x3* invView = RenderConfig::GetInverseViewMatrix();
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)invView, 12);
                SubmitCommandToGeometryFifo(GXFifoCommand_MultiplyMat4x3, (uint32_t*)&worldView.entries, 12);
            }

            SendQueuedDataToGeometryFifo();
            GXFIFO_MATRIX_MODE = 0; // projection
            GXFIFO_MATRIX_PUSH = 0;
            GXFIFO_MATRIX_IDENTITY = 0;
            Matrix4x4 worldViewAgain;
            while (func_020c54fc(&worldViewAgain) != 0) {}
            GXFIFO_MATRIX_POP = 1;
            GXFIFO_MATRIX_MODE = 3; // texture matrix
            SubmitCommandToGeometryFifo(GXFifoCommand_LoadMat4x4, (uint32_t*)worldViewAgain.entries, 16);
            
            // casting to short then unsigned short is necessary to match assembly
            uint16_t xCoordIntPart = (short)((unsigned int)(worldViewAgain.entries[12] >> 4) >> 8);
            uint16_t yCoordIntPart = (short)((unsigned int)(worldViewAgain.entries[13] >> 4) >> 8);
            uint32_t texCoordsPacked = xCoordIntPart | (yCoordIntPart << 16);
            SubmitCommandToGeometryFifo(GXFifoCommand_SetTextureCoords, &texCoordsPacked, 1);
        }
        uint32_t posVectorMode = 2;
        SubmitCommandToGeometryFifo(GXFifoCommand_SetMatrixMode, &posVectorMode, 1);
        uint32_t retrievePos = 30;
        SubmitCommandToGeometryFifo(GXFifoCommand_GetMatrix, &retrievePos, 1);
    }
    handler->instructionPointer_ += 3;
}

void RenderMeshWithMaterial(NSBXXInternalModel* model, unsigned int materialIdx, unsigned int meshIdx, int bind)
{
    uint32_t commandParams[7];
    MaterialRenderData targetData;
    
    fix32_t upScale = model->upScale_;
    if (upScale != 1 << 12)
    {
        uint32_t upScaleParams[3];
        upScaleParams[0] = upScale;
        upScaleParams[1] = upScale;
        upScaleParams[2] = upScale;
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, upScaleParams, 3);
    }

    if (bind && materialIdx < model->numMaterials_)
    {
        NSBXXModelMaterialData* materialData;
        if (model != NULL && model->materialsOffset_ != 0)
            materialData = (NSBXXModelMaterialData*)((intptr_t)model + model->materialsOffset_);
        else
            materialData = NULL;

        NSBXXMaterial* material = materialData->GetMaterialByIndex(materialIdx);

        // if alpha = 0 don't draw
        if ((material->paramPOLYGON_ATTR_ & 0x1f0000) == 0)
            return;
        
        commandParams[0] = COMBINE_GXFIFO_COMMANDS3(
            GXFifoCommand_DiffuseAmbientReflect,
            GXFifoCommand_SpecularReflectEmit,
            GXFifoCommand_SetPolygonAttr);
        commandParams[1] = material->paramDIF_AMB_;
        commandParams[2] = material->paramSPE_EMI_;
        commandParams[3] = material->paramPOLYGON_ATTR_;
        // conditionally don't specify alpha?
        if (material->flags_ & 0x20)
            commandParams[3] &= ~0x001f0000;
        // 2a = TEXIMAGE_PARAMS, 2b = PLTT_BASE
        commandParams[4] = COMBINE_GXFIFO_COMMANDS2(
            GXFifoCommand_SetTexImageParams,
            GXFifoCommand_SetTexturePaletteBase);
        commandParams[5] = material->paramTEXIMAGE_PARAMS_;
        commandParams[6] = material->texturePaletteVRAMOffset_;
        SubmitCommandToGeometryFifo(commandParams[0], &commandParams[1], 6);
        
        if (material->flags_ & 1)
        {
            targetData.flags_ = 8;
            targetData.materialWidth_ = material->width_;
            targetData.materialHeight_ = material->height_;
            targetData.materialxScale_ = material->xScale_;
            targetData.materialyScale_ = material->yScale_;
            intptr_t extraDataAddr = (intptr_t)(material + 1);
            if (material->flags_ & 2)
                targetData.flags_ |= 1;
            else
            {
                NSBXXMaterial::ExtensionData_Bit1* ext = (NSBXXMaterial::ExtensionData_Bit1*)extraDataAddr;
                targetData.extensionScaleX_ = ext->scaleX_;
                targetData.extensionScaleY_ = ext->scaleY_;
                extraDataAddr += sizeof(*ext);
            }

            if (!(material->flags_ & 4))
            {
                NSBXXMaterial::ExtensionData_Bit2* ext = (NSBXXMaterial::ExtensionData_Bit2*)extraDataAddr;
                targetData.rotationSine_ = ext->sine_;
                targetData.rotationCosine_ = ext->cosine_;
                extraDataAddr += sizeof(*ext);
            }
            else
                targetData.flags_ |= 2;

            if (!(material->flags_ & 8))
            {
                NSBXXMaterial::ExtensionData_Bit3* ext = (NSBXXMaterial::ExtensionData_Bit3*)extraDataAddr;
                targetData.translateX_ = ext->translateX_;
                targetData.translateY_ = ext->translateY_;
            }
            else
                targetData.flags_ |= 4;

            if (data_020f1cf8[model->materialCallbackType_])
                data_020f1cf8[model->materialCallbackType_](&targetData);
        }
    }

    if (meshIdx < model->numMeshes_)
    {
        NSBXXNameList* meshList = model->GetMeshList();
        NSBXXMesh* mesh = meshList->GetEntryFromu32Offset_v2<NSBXXMesh>(meshIdx);

        SendRawDataToGeometryFifo(mesh->GetGPUCommands(), mesh->gpuCommandsLength_);
    }

    fix32_t downScale = model->downScale_;
    if (downScale != 1 << 12)
    {
        uint32_t downScaleParams[3];
        downScaleParams[0] = downScale;
        downScaleParams[1] = downScale;
        downScaleParams[2] = downScale;
        SubmitCommandToGeometryFifo(GXFifoCommand_ScaleMatrix, downScaleParams, 3);
    }
}