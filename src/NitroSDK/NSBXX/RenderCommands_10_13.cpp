#include "Graphics/NSBXX/RenderCommands_Common.h"

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