#include "Graphics/NSBXX/NSBXX.h"
#include "Graphics/NSBXX/Animation.h"
#include "Graphics/NSBXX/RenderCommands_Common.h"

#pragma optimize_for_size off

#if defined(jpn)
#define func_020ca390 func_020cbe5c

#define data_020f1c74 data_020f1de0
#endif

extern "C"
{
    // memset 
    void func_020ca390(int value, void* dst, unsigned len);
}

void CalculateBoneMatrixRenderDataFromJAC(NSBXXAnimationJAC* jac, int arg, fix32_t time, BoneMatrixRenderData* bmrd);

void CalculateTranslationAmountFrameAligned(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac);
void CalculateTranslationAmountSmooth(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac);

void CalculateScalingAmountFrameAligned(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac);
void CalculateScalingAmountSmooth(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac);

void CalculateRotationFrameAligned(Matrix3x3* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac);
void CalculateRotationSmooth(Matrix3x3* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac);
bool GetMatrixFromIndex(Matrix3x3* out, intptr_t pivotList, intptr_t basisList, int index);

static inline void CalculateThirdRowByCrossProduct(Matrix3x3* mat)
{
    fix32_t m23 = mat->entries[5]; // yes, this order is actually important
    fix32_t m11 = mat->entries[0];
    fix32_t m21 = mat->entries[3];
    fix32_t m13 = mat->entries[2];
    fix32_t m12 = mat->entries[1];
    fix32_t m22 = mat->entries[4];

    mat->entries[6] = (m12 * m23 - m13 * m22) >> 12;
    mat->entries[7] = (m13 * m21 - m11 * m23) >> 12;
    mat->entries[8] = (m11 * m22 - m12 * m21) >> 12;
}

// processing callback for J.AC animations
extern void (*data_020f1c74)(void*, AnimationData*, int);

void InitializeModelAnimationFromJAC(AnimationData* anim, void* pvJAC, NSBXXInternalModel* model)
{
    NSBXXAnimationJAC* jac = (NSBXXAnimationJAC*)pvJAC;
    anim->pRawData_ = jac;
    anim->callback_ = data_020f1c74;
    anim->numEntries_ = model->numBoneMatrices_;
    func_020ca390(0, anim->entries_, 2 * anim->numEntries_);

    unsigned int trackIdx = 0;
    uint16_t* trackOffsets = jac->trackOffsets_;
    if (trackIdx < jac->numTracks_)
    {
        do
        {
            anim->entries_[trackIdx] = ((NSBXXAnimationJAC::Track*)((intptr_t)jac + trackOffsets[trackIdx]))->flagsAndTargetBoneMatrix_ >> 24 | 0x100;
            trackIdx++;
        } while (trackIdx < jac->numTracks_);
    }
}

void JACAnimationProcessingCallback(void* data, AnimationData* anim, int arg)
{
    BoneMatrixRenderData* boneRenderData = (BoneMatrixRenderData*)data;

    fix32_t time = anim->time_;
    NSBXXAnimationJAC* jac = (NSBXXAnimationJAC*)anim->pRawData_;

    if (time >= jac->numFrames_ << 12)
        time = (jac->numFrames_ << 12) - 1;
    else if (time < 0)
        time = 0;

    CalculateBoneMatrixRenderDataFromJAC(jac, arg, time, boneRenderData);
}

void ApplyBindPoseTranslation(BoneMatrixRenderData* bmrd)
{
    RenderCommandHandler* handler = data_0210a274;
    // this is super cursed, but I guess this executes during command 6 when
    // ip[1] holds the bone index to apply stuff to
    NSBXXBoneMatrix* boneMatrix = handler->boneList_->GetEntryFromu32Offset_v2<NSBXXBoneMatrix>(handler->instructionPointer_[1]);
    if (boneMatrix->flags_ & 1) // no translation data
    {
        bmrd->flags_ |= 4;
    }
    else
    {
        NSBXXBoneMatrix::Translation* tdata = (NSBXXBoneMatrix::Translation*)((intptr_t)boneMatrix + 4);
        bmrd->translate_.x = tdata->x;
        bmrd->translate_.y = tdata->y;
        bmrd->translate_.z = tdata->z;
    }
}

void ApplyBindPoseScaling(BoneMatrixRenderData* bmrd)
{
    RenderCommandHandler* handler = data_0210a274;
    uint8_t* ip = handler->instructionPointer_;
    NSBXXBoneMatrix* boneMatrix = handler->boneList_->GetEntryFromu32Offset_v2<NSBXXBoneMatrix>(ip[1]);
    intptr_t addrScaling = (intptr_t)(boneMatrix + 1);
    unsigned int flags = boneMatrix->flags_;
    if (!(flags & 1)) // has translation data
        addrScaling += sizeof(NSBXXBoneMatrix::Translation);
    if (!(flags & 2))
    {
        if (flags & 8)
            addrScaling += sizeof(NSBXXBoneMatrix::PivotMatrixData);
        else
            addrScaling += sizeof(NSBXXBoneMatrix::RotationMatrixData);
    }
    handler->boneMatrixRenderDataScalePopulateProc_(bmrd, (NSBXXBoneMatrix::Scaling*)addrScaling, ip, flags);
}

// not quite a match, some register nonsense
void ApplyBindPoseRotation(BoneMatrixRenderData* bmrd)
{  
    RenderCommandHandler* handler = data_0210a274;
    NSBXXBoneMatrix* boneMatrix = handler->boneList_->GetEntryFromu32Offset_v2<NSBXXBoneMatrix>(handler->instructionPointer_[1]);

    intptr_t addrRotation = (intptr_t)(boneMatrix + 1);
    if (!(boneMatrix->flags_ & 1)) // has translation data
        addrRotation += sizeof(NSBXXBoneMatrix::Translation);

    if (!(boneMatrix->flags_ & 2)) // has rotation data
    {
        if (boneMatrix->flags_ & 8) // pivot matrix format
        {
            int pivotForm;
            pivotForm = (boneMatrix->flags_ & 0xf0) >> 4;
            NSBXXBoneMatrix::PivotMatrixData* pivot = (NSBXXBoneMatrix::PivotMatrixData*)addrRotation;
            fix32_t entryA = pivot->a;
            fix32_t entryB = pivot->b;

            func_020ca7d0(&bmrd->rotationMatrix_);
            fix32_t unit = (boneMatrix->flags_ & 0x100) ? -1 << 12 : 1 << 12;

            int indexA = data_020e9284[pivotForm].a;
            int indexB = data_020e9284[pivotForm].b;
            
            bmrd->rotationMatrix_.entries[pivotForm] = unit;
            bmrd->rotationMatrix_.entries[indexA] = entryA;
            bmrd->rotationMatrix_.entries[indexB] = entryB;

            fix32_t entryC = (boneMatrix->flags_ & 0x200) ? -entryB : entryB;
            int indexC = data_020e9284[pivotForm].c;
            bmrd->rotationMatrix_.entries[data_020e9284[pivotForm].c] = entryC;

            fix32_t entryD = (boneMatrix->flags_ & 0x400) ? -entryA : entryA;
            int indexD = data_020e9284[pivotForm].d;
            bmrd->rotationMatrix_.entries[indexD] = entryD;
        }
        else // generic 3x3 matrix format
        {
            bmrd->rotationMatrix_.entries[0] = boneMatrix->m_11;
            NSBXXBoneMatrix::RotationMatrixData* rot = (NSBXXBoneMatrix::RotationMatrixData*)addrRotation;
            bmrd->rotationMatrix_.entries[1] = rot->entries[0];
            bmrd->rotationMatrix_.entries[2] = rot->entries[1];
            bmrd->rotationMatrix_.entries[3] = rot->entries[2];
            bmrd->rotationMatrix_.entries[4] = rot->entries[3];
            bmrd->rotationMatrix_.entries[5] = rot->entries[4];
            bmrd->rotationMatrix_.entries[6] = rot->entries[5];
            bmrd->rotationMatrix_.entries[7] = rot->entries[6];
            bmrd->rotationMatrix_.entries[8] = rot->entries[7];
        }
    }
    else // no rotation data
    {
        bmrd->flags_ |= 2;
    }
}

void CalculateBoneMatrixRenderDataFromJAC(NSBXXAnimationJAC* jac, int arg, fix32_t time, BoneMatrixRenderData* bmrd)
{
    NSBXXAnimationJAC::Track* track = (NSBXXAnimationJAC::Track*)((intptr_t)jac + jac->trackOffsets_[arg]);

    NSBXXBoneMatrix::Scaling scaleData;

    unsigned int trackFlags = track->flagsAndTargetBoneMatrix_;

    if (trackFlags & 1) // track has no channels at all
        bmrd->flags_ = 7;
    else
    {
        intptr_t channelAddr = (intptr_t)(track + 1);
        bool smooth;
        if ((time & 0xfff) != 0 && (jac->unk_8 & 1))
            smooth = true;
        else
            smooth = false;

        bmrd->flags_ = 0;
        
        if (!(trackFlags & 6)) // has translation channels (check both bits)
        {
            if (!(trackFlags & 0x08)) // x translation non-const
            {
                if (smooth)
                    CalculateTranslationAmountSmooth(&bmrd->translate_.x, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateTranslationAmountFrameAligned(&bmrd->translate_.x, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                channelAddr += sizeof(NSBXXAnimationJAC::Track::ChannelNonConst);
            }
            else // x-translation constant
            {
                bmrd->translate_.x = *(fix32_t*)(channelAddr);
                channelAddr += sizeof(fix32_t);
            }

            if (!(trackFlags & 0x10)) // y-translation non-const
            {
                if (smooth)
                    CalculateTranslationAmountSmooth(&bmrd->translate_.y, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateTranslationAmountFrameAligned(&bmrd->translate_.y, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                channelAddr += sizeof(NSBXXAnimationJAC::Track::ChannelNonConst);
            }
            else // y-translation constant
            {
                bmrd->translate_.y = *(fix32_t*)(channelAddr);
                channelAddr += sizeof(fix32_t);
            }

            if (!(trackFlags & 0x20)) // z-translation non-const
            {
                if (smooth)
                    CalculateTranslationAmountSmooth(&bmrd->translate_.z, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateTranslationAmountFrameAligned(&bmrd->translate_.z, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                channelAddr += sizeof(NSBXXAnimationJAC::Track::ChannelNonConst);
            }
            else // z-translation constant
            {
                bmrd->translate_.z = *(fix32_t*)(channelAddr);
                channelAddr += sizeof(fix32_t);
            }
        }
        else if (trackFlags & 2)
        {
            bmrd->flags_ |= 4;
        }
        else
        {
            ApplyBindPoseTranslation(bmrd);
        }

        if (!(trackFlags & 0xc0))
        {
            if (!(trackFlags & 0x100))
            {
                if (smooth)
                    CalculateRotationSmooth(&bmrd->rotationMatrix_, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateRotationFrameAligned(&bmrd->rotationMatrix_, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                channelAddr += sizeof(NSBXXAnimationJAC::Track::ChannelNonConst);
            }
            else // rotation channel constant
            {
                if (GetMatrixFromIndex(&bmrd->rotationMatrix_, (intptr_t)jac->GetPivotMatrices(), (intptr_t)jac->GetBasisMatrices(), *(uint32_t*)(channelAddr)))
                {
                    fix32_t m23 = bmrd->rotationMatrix_.entries[5];
                    fix32_t m11 = bmrd->rotationMatrix_.entries[0];
                    fix32_t m21 = bmrd->rotationMatrix_.entries[3];
                    fix32_t m13 = bmrd->rotationMatrix_.entries[2];
                    fix32_t m12 = bmrd->rotationMatrix_.entries[1];
                    fix32_t m22 = bmrd->rotationMatrix_.entries[4];

                    fix32_t m31 = (m12 * m23 - m13 * m22) >> 12;
                    fix32_t m32 = (m13 * m21 - m11 * m23) >> 12;
                    fix32_t m33 = (m11 * m22 - m12 * m21) >> 12;
                    bmrd->rotationMatrix_.entries[6] = m31;
                    bmrd->rotationMatrix_.entries[7] = m32;
                    bmrd->rotationMatrix_.entries[8] = m33;
                }
                channelAddr += sizeof(uint32_t);
            }
        }
        else if (trackFlags & 0x40)
        {
            bmrd->flags_ |= 2;
        }
        else
        {
            ApplyBindPoseRotation(bmrd);
        }

        if (!(trackFlags & 0x600)) // has scaling channels (check both bits)
        {
            // Note scale channels are 8 bytes whether constant or not
            if (!(trackFlags & 0x800)) // x-scaling non-const
            {
                fix32_t xscales[2];
                if (smooth)
                    CalculateScalingAmountSmooth(xscales, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateScalingAmountFrameAligned(xscales, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                scaleData.x = xscales[0];
                scaleData.x_v2 = xscales[1];
            }
            else
            {
                scaleData.x = *(fix32_t*)channelAddr;
                scaleData.x_v2 = *(fix32_t*)(channelAddr + 4);
            }
            channelAddr += 8;

            if (!(trackFlags & 0x1000)) // y-scaling non-const
            {
                fix32_t yscales[2];
                if (smooth)
                    CalculateScalingAmountSmooth(yscales, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateScalingAmountFrameAligned(yscales, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                scaleData.y = yscales[0];
                scaleData.y_v2 = yscales[1];
            }
            else
            {
                scaleData.y = *(fix32_t*)(channelAddr);
                scaleData.y_v2 = *(fix32_t*)(channelAddr + 4);
            }
            channelAddr += 8;

            if (!(trackFlags & 0x2000)) // z-scaling non-const
            {
                fix32_t zscales[2];
                if (smooth)
                    CalculateScalingAmountSmooth(zscales, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                else
                    CalculateScalingAmountFrameAligned(zscales, time, (NSBXXAnimationJAC::Track::ChannelNonConst*)channelAddr, jac);
                scaleData.z = zscales[0];
                scaleData.z_v2 = zscales[1];
            }
            else
            {
                scaleData.z = *(fix32_t*)(channelAddr);
                scaleData.z_v2 = *(fix32_t*)(channelAddr + 4);
            }
        }
        else if (trackFlags & 0x200)
        {
            bmrd->flags_ |= 1;
        }
        else
        {
            ApplyBindPoseScaling(bmrd);
            return;
        }
    }

    int boneMatrixFlags = (bmrd->flags_ & 1) ? 4 : 0;
    data_0210a274->boneMatrixRenderDataScalePopulateProc_(bmrd, &scaleData, data_0210a274->instructionPointer_, boneMatrixFlags);
}

void CalculateTranslationAmountFrameAligned(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac)
{
    unsigned int highWeightFrame, lowWeightFrame;
    unsigned int metadata;
    unsigned int thisFrame = time >> 12;
    fix32_t* samples = (fix32_t*)((intptr_t)jac + channel->samplesOffset_);
    fix16_t* samples16 = (fix16_t*)samples;

    unsigned int averageIndex, directIndex = thisFrame;

    metadata = channel->metadata_;
    // log-rate = 0: every frame is sampled, no need for averaging
    if ((metadata & 0xc0000000) == 0)
        goto compute_directly;

{
    unsigned int endFrame = (metadata & 0x1fff0000) >> 16;

    // log-rate = 1 or 3: case 3 ignored, so only case 1 (sample every 2nd frame)
    if (metadata & 0x40000000)
    {
        if (thisFrame & 1) // odd frame, need to average
        {
            if (thisFrame > endFrame)
            {
                directIndex = (endFrame / 2) + 1;
                goto compute_directly;
            }
            else
            {
                averageIndex = thisFrame / 2;
                goto compute_average;
            }
        }
        else
        {
            directIndex = thisFrame / 2;
            goto compute_directly;
        }
    }
    else // log-rate = 2, sample every 4th frame
    {
        unsigned int mod4 = thisFrame & 3;
        if (mod4 != 0)
        {
            if (thisFrame > endFrame)
            {
                directIndex = (endFrame / 4) + mod4;
                goto compute_directly;
            }
            else if (thisFrame & 1) // odd frame: interpolate with 1/4, 3/4 or vice versa
            {
                if (thisFrame & 2) // 3 mod 4: put weight on the later frame
                {
                    highWeightFrame = (thisFrame / 4) + 1;
                    lowWeightFrame = thisFrame / 4;
                }
                else // 1 mod 4: put weight on the earlier frame
                {
                    lowWeightFrame = (thisFrame / 4) + 1;
                    highWeightFrame = thisFrame / 4;
                }

                if (metadata & 0x20000000) // 16-bit samples
                {
                    fix32_t rescaledHigh = samples16[highWeightFrame] * 3;
                    *out = (rescaledHigh + samples16[lowWeightFrame]) >> 2;
                }
                else
                {
                    int64_t high = samples[highWeightFrame];
                    *out = (high * 3 + samples[lowWeightFrame]) >> 2;
                }
                return;
            }
            else // 2 mod 4: can do 50/50 averaging
            {
                averageIndex = thisFrame / 4;
                goto compute_average;
            }
        }
        else
        {
            directIndex = thisFrame / 4;
            goto compute_directly;
        }
    }
}

compute_average:
    if (metadata & 0x20000000) // 16-bit samples
        *out = (*(samples16 + averageIndex) + *(samples16 + averageIndex + 1)) >> 1;
    else // 32-bit samples
        *out = (*(samples + averageIndex) >> 1) + (*(samples + averageIndex + 1) >> 1);
    return;
compute_directly:
    if (metadata & 0x20000000) // 16-bit samples
        *out = samples16[directIndex];
    else
        *out = samples[directIndex];
}

// note: this matches with sp1p5 or pre sp2p2 but does not match with sp2p2
// combined with the fact we need O2,p I think this was compiled separately
void CalculateTranslationAmountSmooth(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac)
{
    unsigned int frameIdx = time >> 12;
    fix32_t* samples = (fix32_t*)((intptr_t)jac + channel->samplesOffset_);
    fix16_t* samples16 = (fix16_t*)samples;
    unsigned int flags = channel->metadata_;

    // at end of animation
    if (jac->numFrames_ - 1 == frameIdx)
    {
        unsigned int sampleIdx = frameIdx;
        if (flags & 0xc0000000) // log-rate != 0, i.e. samples less than once per frame
        {
            if (flags & 0x40000000) // log-rate = 1
            {
                sampleIdx = (frameIdx & 1) + (frameIdx >> 1);
            }
            else // log-rate = 2
            {
                sampleIdx = (frameIdx & 3) + (frameIdx >> 2);
            }
        }

        if (jac->unk_8 & 2) // probably means animation is looping?
        {
            fix32_t timeFraction = time & 0xfff;
            fix32_t lerp0;
            fix32_t lerp1;
            if (flags & 0x20000000) // 16-bit samples
            {
                lerp0 = samples16[sampleIdx];
                lerp1 = samples16[0];
            }
            else
            {
                lerp0 = samples[sampleIdx];
                lerp1 = samples[0];
            }
            *out = lerp0 + ((timeFraction * (lerp1 - lerp0)) >> 12);
            return;
        }
        else
        {
            fix32_t finalValue;
            if (flags & 0x20000000) // 16-bit sample
            {
                finalValue = samples16[sampleIdx];
            }
            else
                finalValue = samples[sampleIdx];
            *out = finalValue;
            return;
        }
    }

    int preMultiply;
    int rightShift;

    if ((flags & 0xc0000000) != 0)
    {
        unsigned int endFrame = (flags & 0x1fff0000) >> 0x10;
        if (flags & 0x40000000) // log-rate = 1
        {
            if (frameIdx >= endFrame)
                frameIdx = endFrame / 2;
            else
            {
                frameIdx = frameIdx / 2;
                time &= 0x1fff;
                preMultiply = 2;
                rightShift = 1;
                goto skip_default_values;
            }
        }
        else // log-rate 2
        {
            if (frameIdx >= endFrame)
                frameIdx = (frameIdx & 3) + (frameIdx / 4);
            else
            {
                frameIdx = frameIdx / 4;
                time &= 0x3fff;
                preMultiply = 4;
                rightShift = 2;
                goto skip_default_values;
            }               
        }
    }
default_values:
    time &= 0xfff;
    preMultiply = 1;
    rightShift = 0;
skip_default_values:
    fix32_t lerp0;
    fix32_t lerp1;
    if (flags & 0x20000000) // 16-bit samples
    {
        lerp0 = (samples16 + frameIdx)[0];
        lerp1 = (samples16 + frameIdx)[1];
    }
    else
    {
        lerp0 = (samples + frameIdx)[0];
        lerp1 = (samples + frameIdx)[1];
    }

    *out = (lerp0 * preMultiply + ((time * (lerp1 - lerp0)) >> 12)) >> rightShift;
}

void CalculateScalingAmountFrameAligned(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac)
{
    unsigned int metadata;   
    unsigned int thisFrame = time >> 12;
    
    NSBXXAnimationJAC::ScaleSample32* samples = (NSBXXAnimationJAC::ScaleSample32*)((intptr_t)jac + channel->samplesOffset_);
    NSBXXAnimationJAC::ScaleSample16* samples16 = (NSBXXAnimationJAC::ScaleSample16*)samples;

    metadata = channel->metadata_;
    
    unsigned int averageIdx;
    
    if ((metadata & 0xc0000000) == 0)
        goto compute_directly;

{
    unsigned int endFrame = (metadata & 0x1fff0000) >> 16;

    // log-rate = 1 or 3: case 3 ignored, so only case 1 (sample every 2nd frame)
    if (metadata & 0x40000000)
    {
        if (thisFrame & 1) // odd frame, need to average
        {
            if (thisFrame > endFrame)
            {
                thisFrame = (endFrame / 2) + 1;
                goto compute_directly;
            }
            else
            {
                averageIdx = thisFrame / 2;
                goto compute_average;
            }
        }
        else
        {
            thisFrame = thisFrame / 2;
            goto compute_directly;
        }
    }
    else // log-rate = 2, sample every 4th frame
    {
        unsigned int mod4 = thisFrame & 3;
        if (mod4 != 0)
        {
            if (thisFrame > endFrame)
            {
                // why do we add mod4 here?
                thisFrame = (endFrame / 4) + mod4;
                goto compute_directly;
            }
            else if (thisFrame & 1) // odd frame: interpolate with 1/4, 3/4 or vice versa
            {
                unsigned int highWeightFrame;
                unsigned int lowWeightFrame;
                if (thisFrame & 2) // 3 mod 4: put weight on the later frame
                {
                    highWeightFrame = (thisFrame / 4) + 1;
                    lowWeightFrame = thisFrame / 4;
                }
                else // 1 mod 4: put weight on the earlier frame
                {
                    lowWeightFrame = (thisFrame / 4) + 1;
                    highWeightFrame = thisFrame / 4;
                }

                if (metadata & 0x20000000) // 16-bit samples
                {
                    fix32_t sumPrim = samples16[highWeightFrame].primary_;
                    fix32_t lowPrim = samples16[lowWeightFrame].primary_;
                    sumPrim = sumPrim * 3 + lowPrim;
                    out[0] = sumPrim >> 2;
                    fix32_t sumSec = samples16[highWeightFrame].secondary_;
                    fix32_t lowSec = samples16[lowWeightFrame].secondary_;
                    sumSec = sumSec * 3 + lowSec;
                    out[1] = sumSec >> 2;
                    return;
                }
                else
                {
                    int64_t highPrim = (int64_t)samples[highWeightFrame].primary_ * 3;
                    int64_t lowPrim = samples[lowWeightFrame].primary_;
                    // this assignment is important...
                    int64_t sumPrim = lowPrim;
                    sumPrim = highPrim + lowPrim;
                    out[0] = sumPrim >> 2;
                    int64_t highSec = (int64_t)samples[highWeightFrame].secondary_ * 3;
                    out[1] = (highSec + samples[lowWeightFrame].secondary_) >> 2;
                    return;
                }
            }
            else // 2 mod 4: can do 50/50 averaging
            {
                averageIdx = thisFrame / 4;
                goto compute_average;
            }
        }
        else
        {
            thisFrame = thisFrame / 4;
            goto compute_directly;
        }
    }
}
    
compute_directly:
    if (metadata & 0x20000000)
    {
        out[0] = samples16[thisFrame].primary_;
        out[1] = samples16[thisFrame].secondary_;
    }
    else
    {
        out[0] = samples[thisFrame].primary_;
        out[1] = samples[thisFrame].secondary_;
    }
    return;
compute_average:
    if (metadata & 0x20000000)
    {
        out[0] = ((samples16 + averageIdx)->primary_ + (samples16 + averageIdx + 1)->primary_) >> 1;
        out[1] = ((samples16 + averageIdx)->secondary_ + (samples16 + averageIdx + 1)->secondary_) >> 1;
    }
    else
    {
        out[0] = ((samples + averageIdx)[0].primary_ + (samples + averageIdx)[1].primary_) >> 1;
        out[1] = ((samples + averageIdx)[0].secondary_ + (samples + averageIdx)[1].secondary_) >> 1;
    }
}

void CalculateScalingAmountSmooth(fix32_t* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac)
{
    NSBXXAnimationJAC::ScaleSample32* samples = (NSBXXAnimationJAC::ScaleSample32*)((intptr_t)jac + channel->samplesOffset_);
    NSBXXAnimationJAC::ScaleSample16* samples16 = (NSBXXAnimationJAC::ScaleSample16*)samples;
    unsigned int flags = channel->metadata_;
    unsigned int frameIdx = time >> 12;
    unsigned int lerpEndIdx;

    int preMultiply;
    int rightShift;

    // at end of animation
    if (jac->numFrames_ - 1 == frameIdx)
    {
        if (flags & 0xc0000000) // log-rate != 0, i.e. samples less than once per frame
        {
            if (flags & 0x40000000) // log-rate = 1
            {
                frameIdx = (frameIdx & 1) + (frameIdx >> 1);
            }
            else // log-rate = 2
            {
                frameIdx = (frameIdx & 3) + (frameIdx >> 2);
            }
        }

        if (jac->unk_8 & 2)// probably means animation is looping?
        {
            lerpEndIdx = 0;
        }
        else
        {
            if (flags & 0x20000000) // 16-bit samples
            {
                out[0] = samples16[frameIdx].primary_;
                out[1] = samples16[frameIdx].secondary_;
            }
            else
            {
                out[0] = samples[frameIdx].primary_;
                out[1] = samples[frameIdx].secondary_;
            }
            return;
        }       
    }
    else if ((flags & 0xc0000000) != 0)
    {
        unsigned int endFrame = (flags & 0x1fff0000) >> 0x10;
        if (flags & 0x40000000) // log-rate = 1
        {
            if (frameIdx >= endFrame)
            {
                frameIdx = endFrame / 2;
                lerpEndIdx = frameIdx + 1;
            }
            else
            {
                frameIdx = frameIdx / 2;
                lerpEndIdx = frameIdx + 1;
                time &= 0x1fff;
                preMultiply = 2;
                rightShift = 1;
                goto skip_default_values;
            }
        }
        else // log-rate 2
        {
            if (frameIdx >= endFrame)
            {
                frameIdx = (frameIdx & 3) + (frameIdx / 4);
                lerpEndIdx = frameIdx + 1;
            }               
            else
            {
                frameIdx = frameIdx / 4;
                lerpEndIdx = frameIdx + 1;
                time &= 0x3fff;
                preMultiply = 4;
                rightShift = 2;
                goto skip_default_values;
            }
        }
    }
    else
        lerpEndIdx = frameIdx + 1;

default_values:
    time &= 0xfff;
    preMultiply = 1;
    rightShift = 0;
skip_default_values:
    fix32_t primaryLerp0;
    fix32_t primaryLerp1;
    fix32_t secondaryLerp0;
    fix32_t secondaryLerp1;
    if (flags & 0x20000000) // 16-bit samples
    {
        primaryLerp0 = samples16[frameIdx].primary_;
        secondaryLerp0 = samples16[frameIdx].secondary_;
        primaryLerp1 = samples16[lerpEndIdx].primary_;
        secondaryLerp1 = samples16[lerpEndIdx].secondary_;
    }
    else
    {
        primaryLerp0 = samples[frameIdx].primary_;
        secondaryLerp0 = samples[frameIdx].secondary_;
        primaryLerp1 = samples[lerpEndIdx].primary_;
        secondaryLerp1 = samples[lerpEndIdx].secondary_;
    }

    out[0] = (primaryLerp0 * preMultiply + ((time * (primaryLerp1 - primaryLerp0)) >> 12)) >> rightShift;
    out[1] = (secondaryLerp0 * preMultiply + ((time * (secondaryLerp1 - secondaryLerp0)) >> 12)) >> rightShift;
}

void CalculateRotationFrameAligned(Matrix3x3* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac)
{
    unsigned int flags = channel->metadata_;
    unsigned int frameIdx = time >> 12;
    unsigned int averageIdx;
    
    unsigned int basisOffset;
    unsigned int pivotOffset = jac->pivotDataOffset_;
    basisOffset = jac->basisMatricesOffset_;
    uint16_t* samples = (uint16_t*)((intptr_t)jac + channel->samplesOffset_);

    if ((flags & 0xc0000000) == 0)
        goto calculate_exact_frame;
    
{
    unsigned int endFrame = (flags & 0x1fff0000) >> 16;
    if (flags & 0x40000000) // log(rate) = 1, sample every second frame
    {
        if (frameIdx & 1)
        {
            if (frameIdx > endFrame)
            {
                frameIdx = (endFrame / 2) + 1;
                goto calculate_exact_frame;
            }
            else
            {
                averageIdx = (frameIdx / 2);
                goto calculate_by_average;
            }
        }
        else
        {
            frameIdx = frameIdx / 2;
            goto calculate_exact_frame;
        }
    }
    else
    {
        unsigned int mod4 = frameIdx & 3;
        if (mod4 != 0)
        {
            if (frameIdx > endFrame)
            {
                frameIdx = mod4 + (endFrame / 4);
                goto calculate_exact_frame;
            }
            else if (frameIdx & 1)
            {
                unsigned int lowWeightFrame;
                unsigned int highWeightFrame;
                int anyBasisMatrix = 0;
                if (frameIdx & 2)
                {
                    lowWeightFrame = frameIdx / 4;
                    highWeightFrame = lowWeightFrame + 1;
                }
                else
                {
                    highWeightFrame = frameIdx / 4;
                    lowWeightFrame = highWeightFrame + 1;
                }

                
                anyBasisMatrix |= GetMatrixFromIndex(out, (intptr_t)jac + pivotOffset,
                    (intptr_t)jac + basisOffset, samples[highWeightFrame]);
                Matrix3x3 tempMatrix;
                anyBasisMatrix |= GetMatrixFromIndex(&tempMatrix, (intptr_t)jac + pivotOffset,
                    (intptr_t)jac + basisOffset, samples[lowWeightFrame]);

                out->entries[0] = tempMatrix.entries[0] + out->entries[0] * 3;
                out->entries[1] = tempMatrix.entries[1] + out->entries[1] * 3;
                out->entries[2] = tempMatrix.entries[2] + out->entries[2] * 3;
                out->entries[3] = tempMatrix.entries[3] + out->entries[3] * 3;
                out->entries[4] = tempMatrix.entries[4] + out->entries[4] * 3;
                out->entries[5] = tempMatrix.entries[5] + out->entries[5] * 3;
                Vector3fix_Normalize(&out->rows[0], &out->rows[0]);
                Vector3fix_Normalize(&out->rows[1], &out->rows[1]);
                if (!anyBasisMatrix)
                {
                    out->entries[6] = tempMatrix.entries[6] + out->entries[6] * 3;
                    out->entries[7] = tempMatrix.entries[7] + out->entries[7] * 3;
                    out->entries[8] = tempMatrix.entries[8] + out->entries[8] * 3;
                    Vector3fix_Normalize(&out->rows[2], &out->rows[2]);
                }
                else
                    CalculateThirdRowByCrossProduct(out);
                return;
            }
            else // frameIdx is even, i.e. 2 mod 4, can do 50/50 average
            {
                averageIdx = frameIdx / 4;
                goto calculate_by_average;
            }
        }
        else
        {
            frameIdx = frameIdx / 4;
            goto calculate_exact_frame;
        }
    }
}

calculate_by_average:
{
    Matrix3x3 tempMatrix;
    int anyBasisMatrix = 0;
    anyBasisMatrix |= GetMatrixFromIndex(out, (intptr_t)jac + pivotOffset,
        (intptr_t)jac + basisOffset, (samples + averageIdx)[0]);
    anyBasisMatrix |= GetMatrixFromIndex(&tempMatrix, (intptr_t)jac + pivotOffset,
        (intptr_t)jac + basisOffset, (samples + averageIdx)[1]);

    out->entries[0] += tempMatrix.entries[0];
    out->entries[1] += tempMatrix.entries[1];
    out->entries[2] += tempMatrix.entries[2];
    out->entries[3] += tempMatrix.entries[3];
    out->entries[4] += tempMatrix.entries[4];
    out->entries[5] += tempMatrix.entries[5];
    Vector3fix_Normalize(&out->rows[0], &out->rows[0]);
    Vector3fix_Normalize(&out->rows[1], &out->rows[1]);

    if (!anyBasisMatrix) // both pivot matrices
    {
        out->entries[6] += tempMatrix.entries[6];
        out->entries[7] += tempMatrix.entries[7];
        out->entries[8] += tempMatrix.entries[8];
        Vector3fix_Normalize(&out->rows[2], &out->rows[2]);
    }
    else
        CalculateThirdRowByCrossProduct(out);

    return;
}
calculate_exact_frame:
{
    bool isBasis = GetMatrixFromIndex(out, (intptr_t)jac + pivotOffset,
        (intptr_t)jac + basisOffset, samples[frameIdx]);
    if (isBasis)
        CalculateThirdRowByCrossProduct(out);
    else
        Vector3fix_Normalize(&out->rows[2], &out->rows[2]);
}
}

void CalculateRotationSmooth(Matrix3x3* out, fix32_t time, NSBXXAnimationJAC::Track::ChannelNonConst* channel, NSBXXAnimationJAC* jac)
{
    unsigned int basisOffset, pivotOffset;
    pivotOffset = jac->pivotDataOffset_;
    basisOffset = jac->basisMatricesOffset_;
    unsigned int flags = channel->metadata_;
    
    unsigned int frameBIdx;
    unsigned int frameAIdx;
    fix32_t reducedTime;
    int preMultiply;
    uint16_t* samples = (uint16_t*)((intptr_t)jac + channel->samplesOffset_);
    
    frameAIdx = time >> 12;

    if (frameAIdx == jac->numFrames_ - 1)
    {
        if ((flags & 0xc0000000) != 0)
        {
            if (flags & 0x40000000)
            {
                frameAIdx = (frameAIdx & 1) + (frameAIdx / 2);
            }
            else
            {
                frameAIdx = (frameAIdx & 3) + (frameAIdx / 4);
            }
        }
        if (jac->unk_8 & 2)
        {
            frameBIdx = 0;
        }
        else
        {
            bool isBasis = GetMatrixFromIndex(out, (intptr_t)jac + pivotOffset,
                (intptr_t)jac + basisOffset, samples[frameAIdx]);
            if (isBasis)
                CalculateThirdRowByCrossProduct(out);
            else
                Vector3fix_Normalize(&out->rows[2], &out->rows[2]);
            return;    
        }
    }
    else if ((flags & 0xc0000000) != 0)
    {
        unsigned int endFrame = (flags & 0x1fff0000) >> 16;
        if (flags & 0x40000000)
        {
            if (frameAIdx >= endFrame)
            {
                frameAIdx = endFrame / 2;
                frameBIdx = frameAIdx + 1;
            }
            else
            {
                frameAIdx = frameAIdx / 2;
                frameBIdx = frameAIdx + 1;
                reducedTime = time & 0x1fff;
                preMultiply = 2;
                goto skip_default_values;
            }
        }
        else
        {
            if (frameAIdx >= endFrame)
            {
                frameAIdx = (frameAIdx & 3) + (frameAIdx / 4);
                frameBIdx = frameAIdx + 1;
            }
            else
            {
                frameAIdx = frameAIdx / 4;
                frameBIdx = frameAIdx + 1;
                reducedTime = time & 0x3fff;
                preMultiply = 4;
                goto skip_default_values;
            }
        }
    }
    else
    {
        frameBIdx = frameAIdx + 1;
    }

default_values:
    reducedTime = time & 0xfff;
    preMultiply = 1;
skip_default_values:
    int anyBasisMatrices = 0;
    Matrix3x3 matrixA;
    Matrix3x3 matrixB;

    anyBasisMatrices |= GetMatrixFromIndex(&matrixA, (intptr_t)jac + pivotOffset,
        (intptr_t)jac + basisOffset, samples[frameAIdx]);
    anyBasisMatrices |= GetMatrixFromIndex(&matrixB, (intptr_t)jac + pivotOffset,
        (intptr_t)jac + basisOffset, samples[frameBIdx]);

    out->entries[0] = matrixA.entries[0] * preMultiply + ((reducedTime * (matrixB.entries[0] - matrixA.entries[0])) >> 12);
    out->entries[1] = matrixA.entries[1] * preMultiply + ((reducedTime * (matrixB.entries[1] - matrixA.entries[1])) >> 12);
    out->entries[2] = matrixA.entries[2] * preMultiply + ((reducedTime * (matrixB.entries[2] - matrixA.entries[2])) >> 12);
    out->entries[3] = matrixA.entries[3] * preMultiply + ((reducedTime * (matrixB.entries[3] - matrixA.entries[3])) >> 12);
    out->entries[4] = matrixA.entries[4] * preMultiply + ((reducedTime * (matrixB.entries[4] - matrixA.entries[4])) >> 12);
    out->entries[5] = matrixA.entries[5] * preMultiply + ((reducedTime * (matrixB.entries[5] - matrixA.entries[5])) >> 12);
    Vector3fix_Normalize(&out->rows[0], &out->rows[0]);
    Vector3fix_Normalize(&out->rows[1], &out->rows[1]);
    if (!anyBasisMatrices)
    {
        out->entries[6] = matrixA.entries[6] * preMultiply + ((reducedTime * (matrixB.entries[6] - matrixA.entries[6])) >> 12);
        out->entries[7] = matrixA.entries[7] * preMultiply + ((reducedTime * (matrixB.entries[7] - matrixA.entries[7])) >> 12);
        out->entries[8] = matrixA.entries[8] * preMultiply + ((reducedTime * (matrixB.entries[8] - matrixA.entries[8])) >> 12);
        Vector3fix_Normalize(&out->rows[2], &out->rows[2]);
    }
    else
        CalculateThirdRowByCrossProduct(out);
}

bool GetMatrixFromIndex(Matrix3x3* out, intptr_t pivotList, intptr_t basisList, int index)
{
    if (index & 0x8000) // pivot matrix
    {
        out->entries[0] = out->entries[1] = out->entries[2] =
        out->entries[3] = out->entries[4] = out->entries[5] = 
        out->entries[6] = out->entries[7] = out->entries[8] = 0;
        NSBXXAnimationJAC::PivotMatrix* pivot = (NSBXXAnimationJAC::PivotMatrix*)(pivotList + (((index & 0x7fff) * 3) << 1));
        fix32_t entryA = pivot->a;
        fix32_t entryB = pivot->b;
        
        int form = pivot->flags & 0xf;
        out->entries[form] = (pivot->flags & 0x10) ? -1 << 12 : 1 << 12;

        out->entries[data_020e9284[form].a] = entryA;
        out->entries[data_020e9284[form].b] = entryB;

        fix32_t entryC = (pivot->flags & 0x20) ? -entryB : entryB;
        out->entries[data_020e9284[form].c] = entryC;
        
        fix32_t entryD = (pivot->flags & 0x40) ? -entryA : entryA;
        out->entries[data_020e9284[form].d] = entryD;

        return false;
    }
    else // basis matrix
    {
        NSBXXAnimationJAC::BasisMatrix* basis = (NSBXXAnimationJAC::BasisMatrix*)(basisList + (((index & 0x7fff) * 5) << 1));
        short accumulation = 0;

        int d4 = basis->data[4];
        out->entries[4] = d4 >> 3;
        accumulation = (accumulation << 3) | (d4 & 7);

        int d0 = basis->data[0];
        out->entries[0] = d0 >> 3;
        accumulation = (accumulation << 3) | (d0 & 7);

        int d1 = basis->data[1];
        out->entries[1] = d1 >> 3;
        accumulation = (accumulation << 3) | (d1 & 7);

        int d2 = basis->data[2];
        out->entries[2] = d2 >> 3;
        accumulation = (accumulation << 3) | (d2 & 7);

        int d3 = basis->data[3];
        out->entries[3] = d3 >> 3;
        accumulation = (accumulation << 3) | (d3 & 7);
        
        out->entries[5] = (accumulation << 19) >> 19; // limit to 13 bits

        return true;
    }
}