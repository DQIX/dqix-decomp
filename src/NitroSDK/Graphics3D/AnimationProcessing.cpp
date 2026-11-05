#include "NitroSDK/Graphics3D/Animation.h"

#if defined(jpn)
#define func_020ca408 func_020cbed4
#define func_020ca458 func_020cbf24
#endif

extern "C"
{
    void func_020ca408(const void* src, void* dst, unsigned len);
    void func_020ca458(int value, void* data, unsigned len);
}

bool Nitro3Di_DefaultProcessMaterialAnimations(MaterialRenderData* material, Nitro3DAnimation* anim, unsigned int matIdx)
{
    bool success = false;
    if (anim != NULL)
    {
        do
        {
            if (matIdx < anim->numEntries && ((anim->entries[matIdx] & 0x300) == 0x100))
            {
                if (anim->callback != NULL)
                {
                    // points to {MAM, MAT, MPT}AnimationProcessingCallback
                    anim->callback(material, anim, anim->entries[matIdx] & 0xff);
                    success = true;
                }
            }
            anim = anim->pNext;
        } while (anim != NULL);
    }
    return success;
}

static void AccumulateScaleVector(BoneMatrixRenderData::Scale* accumulator, const BoneMatrixRenderData::Scale* source, fix32_t scale, int useIdentity)
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

bool Nitro3Di_DefaultProcessJointAnimations(BoneMatrixRenderData* bone, Nitro3DAnimation* anim, unsigned int boneIdx)
{
    if (anim == NULL)
        return false;

    // handle case of only one animation separately
    if (anim->pNext == NULL)
    {
        if (boneIdx < anim->numEntries)
        {
            if ((anim->entries[boneIdx] & 0x300) == 0x100)
            {
                if (anim->callback == NULL)
                    return false;

                anim->callback(bone, anim, anim->entries[boneIdx] & 0xff);
                return true;
            }
            return false;
        }
        return false;
    }

    
    fix32_t totalWeight = 0;
    Nitro3DAnimation* loopAnim;
    int numContributingAnims = 0;
    int numAnimsProcessed = 0;
    loopAnim = anim;
    Nitro3DAnimation* finalAnim = anim; // after the following loop, will hold the last anim to contribute
    do {
        if (boneIdx < loopAnim->numEntries && (loopAnim->entries[boneIdx] & 0x300) == 0x100)
        {
            if (loopAnim->weight > (1 << 12))
                totalWeight += 1 << 12;
            else if (loopAnim->weight > 0)
                totalWeight += loopAnim->weight;
            numContributingAnims++;
            finalAnim = loopAnim;
        }
        loopAnim = loopAnim->pNext;
    } while (loopAnim != NULL);

    if (totalWeight == 0)
    {
        return false;
    }

    if (numContributingAnims == 1)
    {
        int arg = finalAnim->entries[boneIdx];
        if (finalAnim->callback == NULL)
            return false;
        finalAnim->callback(bone, finalAnim, arg & 0xff);
        return true;
    }

    // Multiple animations to deal with
    func_020ca458(0, bone, sizeof(BoneMatrixRenderData));
    bone->flags = 0xffffffff;

    Vector3fix rootRotationTopRow;
    Vector3fix rootRotationBottomRow;

    do
    {
        if (boneIdx < anim->numEntries && (anim->entries[boneIdx] & 0x300) == 0x100 &&
            anim->weight > 0 && anim->callback != NULL)
        {
            BoneMatrixRenderData tempData;
            anim->callback(&tempData, anim, anim->entries[boneIdx] & 0xff);
            if (numAnimsProcessed == 0)
            {
                func_020ca408(&tempData.rotationMatrix.rows[0], &rootRotationTopRow, sizeof(Vector3fix));
                func_020ca408(&tempData.rotationMatrix.rows[2], &rootRotationBottomRow, sizeof(Vector3fix));
            }

            fix32_t thisIntensity = (totalWeight == 0x1000) ? anim->weight : fix32_Divide(anim->weight, totalWeight);
            AccumulateScaleVector(&bone->scale_v0, &tempData.scale_v0, thisIntensity, tempData.flags & 1);
            AccumulateScaleVector(&bone->scale_v1, &tempData.scale_v1, thisIntensity, tempData.flags & 8);
            AccumulateScaleVector(&bone->scale_v2, &tempData.scale_v2, thisIntensity, tempData.flags & 0x10);

            if (!(tempData.flags & 4)) // translation
            {
                bone->translate.x += (fix32_t)(((int64_t)thisIntensity * tempData.translate.x) >> 12);
                bone->translate.y += (fix32_t)(((int64_t)thisIntensity * tempData.translate.y) >> 12);
                bone->translate.z += (fix32_t)(((int64_t)thisIntensity * tempData.translate.z) >> 12);
            }

            if (!(tempData.flags & 2)) // rotation
            {
                // Only accumulate the top two rows, we'll compute the third row (and
                // apply small corrections) later to ensure we get an orthogonal matrix
                bone->rotationMatrix.entries[0] += ((thisIntensity * tempData.rotationMatrix.entries[0]) >> 12);
                bone->rotationMatrix.entries[1] += ((thisIntensity * tempData.rotationMatrix.entries[1]) >> 12);
                bone->rotationMatrix.entries[2] += ((thisIntensity * tempData.rotationMatrix.entries[2]) >> 12);
                bone->rotationMatrix.entries[3] += ((thisIntensity * tempData.rotationMatrix.entries[3]) >> 12);
                bone->rotationMatrix.entries[4] += ((thisIntensity * tempData.rotationMatrix.entries[4]) >> 12);
                bone->rotationMatrix.entries[5] += ((thisIntensity * tempData.rotationMatrix.entries[5]) >> 12);
            }
            else
            {
                // add scaled multiple of identity matrix
                bone->rotationMatrix.entries[0] += thisIntensity;
                bone->rotationMatrix.entries[4] += thisIntensity;
            }
            // recall that set bits mean 'does not have feature': in order to not have
            // a feature, we need to not have had it previously and not have picked it 
            // up in this iteration, so bitwise AND does the trick
            bone->flags &= tempData.flags;
        }
        anim = anim->pNext;
        numAnimsProcessed++;
    } while (anim != NULL);
    Vector3fix_CrossProduct(&bone->rotationMatrix.rows[0], &bone->rotationMatrix.rows[1], &bone->rotationMatrix.rows[2]);

    if (bone->rotationMatrix.rows[0].x == 0 && bone->rotationMatrix.rows[0].y == 0 && bone->rotationMatrix.rows[0].z == 0)
    {
        func_020ca408(&rootRotationTopRow, &bone->rotationMatrix.rows[0], sizeof(Vector3fix));
    }
    else
    {
        Vector3fix_Normalize(&bone->rotationMatrix.rows[0], &bone->rotationMatrix.rows[0]);
    }

    if (bone->rotationMatrix.rows[2].x == 0 && bone->rotationMatrix.rows[2].y == 0 && bone->rotationMatrix.rows[2].z == 0)
    {
        func_020ca408(&rootRotationBottomRow, &bone->rotationMatrix.rows[2], sizeof(Vector3fix));
    }
    else
    {
        Vector3fix_Normalize(&bone->rotationMatrix.rows[2], &bone->rotationMatrix.rows[2]);
    }

    // set row2 = (row3) x (row1). Previously we set row3 = (row1) x (row2), so
    // that row3 and row1 are orthogonal to each other, then normalized both.
    // This way we guarantee the resulting matrix is orthogonal
    Vector3fix_CrossProduct(&bone->rotationMatrix.rows[2], &bone->rotationMatrix.rows[0], &bone->rotationMatrix.rows[1]);

    return true;
}

bool Nitro3Di_DefaultProcessVisibilityAnimations(int* output, Nitro3DAnimation* anim, unsigned int boneIdx)
{
    bool success = false;
    *output = 0;

    do
    {
        if (boneIdx < anim->numEntries && (anim->entries[boneIdx] & 0x300) == 0x100)
        {
            if (anim->callback != NULL)
            {
                int tempOutput;
                anim->callback(&tempOutput, anim, anim->entries[boneIdx] & 0xff);
                *output |= tempOutput;
                success = true;
            }
        }
        anim = anim->pNext;
    } while (anim != NULL);
    return success;
}