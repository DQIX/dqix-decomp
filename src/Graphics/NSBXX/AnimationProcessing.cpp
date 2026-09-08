#include "Graphics/NSBXX/Animation.h"

#pragma optimize_for_size off

#if defined(jpn)
#define func_020ca408 func_020cbed4
#define func_020ca458 func_020cbf24
#endif

extern "C"
{
    void func_020ca408(const void* src, void* dst, unsigned len);
    void func_020ca458(int value, void* data, unsigned len);
}

bool ProcessMaterialAnimationsOnBoundMaterial(MaterialRenderData* material, AnimationData* anim, unsigned int matIdx)
{
    bool success = false;
    if (anim != NULL)
    {
        do
        {
            if (matIdx < anim->numEntries_ && ((anim->entries_[matIdx] & 0x300) == 0x100))
            {
                if (anim->callback_ != NULL)
                {
                    // points to {MAM, MAT, MPT}AnimationProcessingCallback
                    anim->callback_(material, anim, anim->entries_[matIdx] & 0xff);
                    success = true;
                }
            }
            anim = anim->pNext_;
        } while (anim != NULL);
    }
    return success;
}

void AccumulateScaleVector(BoneMatrixRenderData::Scale* accumulator, const BoneMatrixRenderData::Scale* source, fix32_t scale, int useIdentity)
{
    if (useIdentity)
    {
        accumulator->x += scale;
        accumulator->y += scale;
        accumulator->z += scale;
    }
    else
    {
        accumulator->x += (scale * source->x) >> 12;
        accumulator->y += (scale * source->y) >> 12;
        accumulator->z += (scale * source->z) >> 12;
    }
}

bool ProcessJointAnimationsOnBoneMatrix(BoneMatrixRenderData* bone, AnimationData* anim, unsigned int boneIdx)
{
    if (anim == NULL)
        return false;

    // handle case of only one animation separately
    if (anim->pNext_ == NULL)
    {
        if (boneIdx < anim->numEntries_)
        {
            if ((anim->entries_[boneIdx] & 0x300) == 0x100)
            {
                if (anim->callback_ == NULL)
                    return false;

                anim->callback_(bone, anim, anim->entries_[boneIdx] & 0xff);
                return true;
            }
            else return false;
        }
        else return false;
    }

    
    fix32_t totalWeight = 0;
    AnimationData* loopAnim;
    int numContributingAnims = 0;
    int numAnimsProcessed = 0;
    loopAnim = anim;
    AnimationData* finalAnim = anim; // after the following loop, will hold the last anim to contribute
    do {
        if (boneIdx < loopAnim->numEntries_ && (loopAnim->entries_[boneIdx] & 0x300) == 0x100)
        {
            if (loopAnim->weight_ > (1 << 12))
                totalWeight += 1 << 12;
            else if (loopAnim->weight_ > 0)
                totalWeight += loopAnim->weight_;
            numContributingAnims++;
            finalAnim = loopAnim;
        }
        loopAnim = loopAnim->pNext_;
    } while (loopAnim != NULL);

    if (totalWeight == 0)
    {
        return false;
    }

    if (numContributingAnims == 1)
    {
        int arg = finalAnim->entries_[boneIdx];
        if (finalAnim->callback_ == NULL)
            return false;
        finalAnim->callback_(bone, finalAnim, arg & 0xff);
        return true;
    }

    // Multiple animations to deal with
    func_020ca458(0, bone, sizeof(BoneMatrixRenderData));
    bone->flags_ = 0xffffffff;

    Vector3fix rootRotationTopRow;
    Vector3fix rootRotationBottomRow;

    do
    {
        if (boneIdx < anim->numEntries_ && (anim->entries_[boneIdx] & 0x300) == 0x100 &&
            anim->weight_ > 0 && anim->callback_ != NULL)
        {
            BoneMatrixRenderData tempData;
            anim->callback_(&tempData, anim, anim->entries_[boneIdx] & 0xff);
            if (numAnimsProcessed == 0)
            {
                func_020ca408(&tempData.rotationMatrix_.rows[0], &rootRotationTopRow, sizeof(Vector3fix));
                func_020ca408(&tempData.rotationMatrix_.rows[2], &rootRotationBottomRow, sizeof(Vector3fix));
            }

            fix32_t thisIntensity = (totalWeight == 0x1000) ? anim->weight_ : fix32_Divide(anim->weight_, totalWeight);
            AccumulateScaleVector(&bone->scale_v0_, &tempData.scale_v0_, thisIntensity, tempData.flags_ & 1);
            AccumulateScaleVector(&bone->scale_v1_, &tempData.scale_v1_, thisIntensity, tempData.flags_ & 8);
            AccumulateScaleVector(&bone->scale_v2_, &tempData.scale_v2_, thisIntensity, tempData.flags_ & 0x10);

            if (!(tempData.flags_ & 4)) // translation
            {
                bone->translate_.x += (fix32_t)(((int64_t)thisIntensity * tempData.translate_.x) >> 12);
                bone->translate_.y += (fix32_t)(((int64_t)thisIntensity * tempData.translate_.y) >> 12);
                bone->translate_.z += (fix32_t)(((int64_t)thisIntensity * tempData.translate_.z) >> 12);
            }

            if (!(tempData.flags_ & 2)) // rotation
            {
                // Only accumulate the top two rows, we'll compute the third row (and
                // apply small corrections) later to ensure we get an orthogonal matrix
                bone->rotationMatrix_.entries[0] += ((thisIntensity * tempData.rotationMatrix_.entries[0]) >> 12);
                bone->rotationMatrix_.entries[1] += ((thisIntensity * tempData.rotationMatrix_.entries[1]) >> 12);
                bone->rotationMatrix_.entries[2] += ((thisIntensity * tempData.rotationMatrix_.entries[2]) >> 12);
                bone->rotationMatrix_.entries[3] += ((thisIntensity * tempData.rotationMatrix_.entries[3]) >> 12);
                bone->rotationMatrix_.entries[4] += ((thisIntensity * tempData.rotationMatrix_.entries[4]) >> 12);
                bone->rotationMatrix_.entries[5] += ((thisIntensity * tempData.rotationMatrix_.entries[5]) >> 12);
            }
            else
            {
                // add scaled multiple of identity matrix
                bone->rotationMatrix_.entries[0] += thisIntensity;
                bone->rotationMatrix_.entries[4] += thisIntensity;
            }
            // recall that set bits mean 'does not have feature': in order to not have
            // a feature, we need to not have had it previously and not have picked it 
            // up in this iteration, so bitwise AND does the trick
            bone->flags_ &= tempData.flags_;
        }
        anim = anim->pNext_;
        numAnimsProcessed++;
    } while (anim != NULL);
    Vector3fix_CrossProduct(&bone->rotationMatrix_.rows[0], &bone->rotationMatrix_.rows[1], &bone->rotationMatrix_.rows[2]);

    if (bone->rotationMatrix_.rows[0].x == 0 && bone->rotationMatrix_.rows[0].y == 0 && bone->rotationMatrix_.rows[0].z == 0)
    {
        func_020ca408(&rootRotationTopRow, &bone->rotationMatrix_.rows[0], sizeof(Vector3fix));
    }
    else
    {
        Vector3fix_Normalize(&bone->rotationMatrix_.rows[0], &bone->rotationMatrix_.rows[0]);
    }

    if (bone->rotationMatrix_.rows[2].x == 0 && bone->rotationMatrix_.rows[2].y == 0 && bone->rotationMatrix_.rows[2].z == 0)
    {
        func_020ca408(&rootRotationBottomRow, &bone->rotationMatrix_.rows[2], sizeof(Vector3fix));
    }
    else
    {
        Vector3fix_Normalize(&bone->rotationMatrix_.rows[2], &bone->rotationMatrix_.rows[2]);
    }

    // set row2 = (row3) x (row1). Previously we set row3 = (row1) x (row2), so
    // that row3 and row1 are orthogonal to each other, then normalized both.
    // This way we guarantee the resulting matrix is orthogonal
    Vector3fix_CrossProduct(&bone->rotationMatrix_.rows[2], &bone->rotationMatrix_.rows[0], &bone->rotationMatrix_.rows[1]);

    return true;
}

bool ProcessVisibilityAnimations(int* output, AnimationData* anim, unsigned int boneIdx)
{
    bool success = false;
    *output = 0;

    do
    {
        if (boneIdx < anim->numEntries_ && (anim->entries_[boneIdx] & 0x300) == 0x100)
        {
            if (anim->callback_ != NULL)
            {
                int tempOutput;
                anim->callback_(&tempOutput, anim, anim->entries_[boneIdx] & 0xff);
                *output |= tempOutput;
                success = true;
            }
        }
        anim = anim->pNext_;
    } while (anim != NULL);
    return success;
}