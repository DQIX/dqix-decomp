#pragma once

#include "../Graphics/Model3D.h"
#include "../Graphics/Animation3D.h"
#include "../Resource/BCFG.h"
#include "../Graphics/AlphaTween.h"

fix32_t GetAnimationFrameCountFix32(AnimationData*);

struct AnimationPackage
{
    // determines which implementation of AdvanceAnimations() to use, and
    // various other functions split into cases based on this. A value of
    // 0 seems to be a very barebones animation, used for e.g. zoom animation
    // and opening cupboards, while everything else uses type 1
    unsigned int animationType : 1;
    unsigned int packageID : 8;
    BCFG bcfgData;
    // In type 0, there is one of these structs and the Animation3D pointer
    // points to an array of size 4. In type 1, there are multiple of these
    // structs (corresponding directly to BCFG entries), and each instance
    // has just one Animation3D. (Technically, the CCHR/CMOT loading code allows
    // for creating multiple that work together, e.g. appear.nsbca combined
    // with appear.nsbtp if you want both a skeletal animation and a pattern 
    // animation. But I don't see this used in practice, and most code just treats
    // the reference as being to a single Animation3D).
    //
    // From looking at 0203643c, which is used for loading objects from 
    // .chr files (typically quite simple / small models, such as the ship
    // or certain NPCs), the four Animation3D entries in type 0 are one for
    // each of { nsbca, nsbma, nsbtp, nsbta }. 
    struct Reference
    {
        int count;
        Animation3D* data;
    } *pAnim3Ds;
    AnimationPackage* pNext;

    // Initializes to type 0
    void Reset();
};

// References a *.chr file or similar, maybe a *.mon?
struct ObjectArchiveLoadInfo
{
    int unk_0;
    const void* fileData;
    int unk_8;
    SafeAllocator* allocator;
    int unk_10;
    int unk_14;
    int unk_18;
    int packageID;
};

#define OBJECT_ANIMATION_DONT_LOOP 1
#define OBJECT_ANIMATION_REVERSE 4

// Represents an instance of an object in the world that has a 3D model.
class Object3D
{
public:
    short unknown_0_;
    short unknown_2_;
    short unknown_4_;
    unsigned short unknown_6_;
    Model3D* pModel_;
    AnimationPackage* loadedAnimationPackageList_;
    AnimationPackage* activeAnimationPackage_;
    BCFG::AnimationRecord* activeAnimationRecord_;
    char activeAnimationIndex_;
    unsigned char animationFlags_;
    unsigned char unk_1a[2];

    fix32_t animationTime_;
    fix32_t prevFrameAnimationTime_; // previous value of animationTime?
    fix32_t normalizedAnimationTime_; // between 0 and 1
    fix32_t prevFrameNormalizedAnimationTime_;

    // used e.g. to transition between standing & running
    struct AnimationBlend
    {
        AnimationPackage* source;
        fix32_t sourceTime;
        unsigned short unk_8;
        short blendWeight;
        unsigned int blendTimeRemaining;

        void Clear();
    } priorAnimationBlend_;
    fix32_t defaultAnimBlendDuration_;

    // these flags are temporary - they only tell you if e.g. an animation
    // completed / looped back to the beginning 
    unsigned char animationEnded_ : 1; // set if the animation stopped, i.e. not looping
    unsigned char unknown_40_bit_1_ : 1;
    unsigned char animationReachedEnd_ : 1; // also set for looping animations when returning to start
    // two uses: for a child object, inheritedAlpha is the alpha value of the
    // parent (i.e. compute inherited*own/31 in parent), but for a parent object
    // inheritedAlpha is the field that the calling code can specify and run
    // transitions on. 
    unsigned char inheritedAlpha_ : 5;
    unsigned char ownAlpha_ : 5;
    unsigned char drawSuccessful_ : 1;
    // this is very weird and I don't know where it's used. If flag is set and
    // this is nonzero, this will count down to zero and render the model without
    // textures until that point.
    unsigned char noTextureTimer_;
    char padding_43;
    Vector3fix position_;
    // might not be a vector.
    // As a transformation x applies first, then y then z
    Vector3fix rotation_;
    fix16_t scale_[3];
    char padding_62[2];
    fix32_t radius_;
    fix32_t height_;
    unsigned int flags_;
    AlphaTween alphaTransition_;

    unsigned short unknown_78_;
    unsigned short unknown_7a_; // relates to sound effects somehow
    fix16_t animationPlaybackSpeed_;
    fix16_t targetAnimationPlaybackSpeed_;
    unsigned short animationPlaybackSpeedTransitionRemainingTicks_;
    char padding_82[2];
    // not sure about this, it could be sth like position from which to 
    // project the shadow. from testing the y-coordinate doesn't affect
    // the visible result though. 
    Vector3fix maybeShadowPosition_;
    short shadowBoneIndex_;
    // if this is a child, it will be attached to its parent at this bone (in the parent)
    short attachmentBoneIndex_;

    // slightly weird: this list includes the parent, at the front.
    // in practice I've only seen this used for the player & their weapon, making
    // this more like (parent, child)
    Object3D* pPrevChild_;
    Object3D* pNextChild_;
    short unknown_9c_;
    char unknown_9e_;
    char padding_9f;

    // Only used in drawing, not passed down the hierarchy. Can be > 1 afaik
    // (final alpha value is still capped at 31 though)
    fix16_t alphaScaleFactor_;
    char unk_a2[2];
    unsigned int alternativeTexturePaletteOffset_;
    // looks to be something like tracked bone matrices (a linked list)

    // you can track a particular bone's world matrix using these.
    // The model's render commands get hooked so that any time a matrix is
    // computed for drawing, it gets stored here. (The user creates an instance
    // of this struct and requests to add it to this list).
    struct TrackedBoneMatrix
    {
        unsigned short boneIndex;
        Matrix4x3 matrix;
        TrackedBoneMatrix* pNext;
    } *trackedBoneMatrixList_;

    typedef void (Object3D::*AnimationAdvanceProc)();

public:
    void Initialize();

    // advances the active animation and any in-progress transitions of
    // alpha value or playback rate
    void AdvanceEffects();

    // dynamic dispatch to AdvanceAnimations_v{0 or 1}
    // based on the first bit in unknown_10_
    void AdvanceAnimations();

    void AdvanceAnimations_v0();
    void AdvanceAnimations_v1();

    // commits the currently active animation(s) at the current time for
    // subsequent draw call. if non-null argument is passed, the animations of
    // this object will be applied there instead of to itself
    void ApplyAnimations(Object3D* otherObj);
    void ApplyTextures();
    void PopulateRenderConfigWorld();

    // I don't know what these flags do, but it is a visibility check among other things
    bool IsVisibleAndAdjustFlags();

    // might need to be int return type, but functionally a bool
    bool Draw(bool applyClipping);

    // Used for drawing the doors on cupboards etc (the animated part)
    // Similar to the main Draw function but with much less config
    bool DrawSimple(bool applyClipping);

    // Used to draw the player's head & weapon in the equipment menu,
    // and in a few instances in overlay 23.
    // This version doesn't set the success flag or submit anything
    // to the FIFO
    bool DrawSimple2(bool applyClipping);

    // used in overlay 25
    bool DrawMeshWithMaterial(bool applyClipping, int material, int mesh, int bind);

    // like DrawMeshWithMaterial() but doesn't touch the RenderConfig.
    // used for drawing shadows under objects
    bool DrawMeshWithMaterialSimple(bool applyClipping, int material, int mesh, int bind);

    // used somewhere in overlay 15, not sure what for
    bool DrawSimple3(bool applyClipping);

    // I don't know exactly how this works, but among other things it's used
    // in overlay 6 as part of the procedure for computing where to place the
    // 2D sprite of the item that the alchemy pot spits out. It calls Model3D::Draw()
    // but there might be some trickery to ensure nothing is displayed directly 
    // as a result of this call
    void MaybeUpdateBonePositions();

    void SendTransformToFifo();
    void SendTranslationToFifo();
    void SendRotationToFifo();
    void SendScalingToFifo();

    // A copy of the raw model file will be created, so you can load
    // it into a scratch buffer or similar just to pass it here
    void SetModelFromFileCopy(SafeAllocator* alloc, const void* modelFile,
        unsigned int modelLength, Model3D::TextureStagingMode stagingMode);

    // The raw model file provided will be used, and must persist until
    // this object is destroyed or has its model changed
    bool SetModelFromFile(SafeAllocator* alloc, void* modelFile,
        unsigned int modelLength, Model3D::TextureStagingMode stagingMode);

    void LoadType0AnimationFromFile(int slot, const char* filename, SafeAllocator* alloc);
    // creates its own copy
    void LoadType0AnimationFromFileInMemory(int slot, SafeAllocator* alloc, const void* data, unsigned int len);
    void LoadType0AnimationFromPersistentMemory(int slot, SafeAllocator* alloc, void* data, unsigned int len);

    void LoadType0AnimationPackageFromBCFGScript(const char* filename, SafeAllocator* alloc);
    AnimationPackage* CreateType0AnimationPackage(SafeAllocator* alloc);
    void LoadType0AnimationPackageFromBCFGScript(SafeAllocator* alloc, const void* script, unsigned int len);

    bool LoadFromCHRArchive(ObjectArchiveLoadInfo* loadInfo);
    bool InternalLoadFromCHRArchive(ObjectArchiveLoadInfo* loadInfo);

    // loads an object/character that uses the more complicated 
    // animation system. Not sure about exact file formats, but both cchr
    // and cmot are used for loading monsters
    bool LoadFromCCHROrCMOTArchive(ObjectArchiveLoadInfo* loadInfo, int (*animCallback)(BCFG::AnimationRecord*));

    // Sets the target object's model and position/visual settings.
    // This does not allocate/copy any dynamic memory
    void ShallowCloneTo(Object3D* out);
    void ShallowCloneModelAndAnimationsTo(Object3D* out);

    bool MaybeSetRegularAnimation(const char* animName, int flags);
    bool MaybeSetBCFGAnimation(int index, int flags);
    void StopCurrentAnimation();

    // offset 40 bit 0, is set the frame that an animation goes from
    // playing to stopped, cleared on subsequent frames
    int HasAnimationStopped() const;
    // offset 40 bit 2, is set the frame that an animation reaches the end,
    // either to stop or to loop back to beginning. cleared on subsequent frames
    int HasAnimationReachedEnd() const;
    // offset 40 bit 1, when this bit is set the animation skips advancing for
    // one frame then this bit is cleared.
    int GetOffset40Bit1() const;

    void SetCurrentAnimationTime(fix32_t time);
    fix32_t GetCurrentAnimationTime() const;
    // sets the normalized animation time and calculates the 'true'
    // animation time as appropriate. This is only used in two places: somewhere
    // in overlay 26, apparently to synchronize effects with attacks in combat,
    // and in overlay 6 (renkin) to synchronize the pulsing/flashing effect
    // with the alchemy pot's own animation when alchemizing an item
    void SetNormalizedAnimationTime(fix32_t normTime);

    void TransitionInheritedAlpha(int alpha, int duration);

    // I have no idea what this flag is for
    void SetFlag16();
    void ClearFlag16();

    void Destroy();
    void RemoveAnimationPackageByID(int id);
    void RemoveAllAnimationPackages();

    bool IsVisible() const;
    void MakeVisible();
    void MakeHidden();
    void SetInheritedAlpha(int alpha);
    int GetInheritedAlpha() const;
    int GetOwnAlpha() const;
    int GetCombinedAlpha() const;

    BCFG* GetCurrentAnimationConfig();

    void SetScale(const Vector3fix* vec);
    void SetScale(fix32_t x, fix32_t y, fix32_t z);
    Vector3fix GetScale() const;

    void SetDiffuseColor(unsigned int bgrCol); 
    void UseRenderConfigDiffuseColor();

    BCFG::AnimationRecord* GetLoadedAnimationRecord(const char* name);
    AnimationPackage* GetAnimationPackageByIndex(int idx);

    void SetField06(unsigned short value);
    unsigned short GetField06() const;
    void SetField78(unsigned short value);
    unsigned short GetField78() const;
    void SetField7a(unsigned short value);
    unsigned short GetField7a() const;

    // If transitionTime != 0 then the speed will adjust linearly towards
    // the specified value over that many frames/ticks
    void SetAnimationPlaybackSpeed(fix16_t speed, int transitionTime);

    void StageTextureData();
    void ComputeShadowPosition();

    Vector3fix MaybeGetShadowSource() const;

    void SetHeight(fix32_t height);
    fix32_t GetHeight() const;
    void SetRadius(fix32_t radius);
    fix32_t GetRadius() const;

    bool IsTransitioningAnimations() const;
    // moves straight into the *new* animation
    void SkipAnimationTransition();

    void ClearChildListData();
    
    // attaches this object to the specified object at the specified bone.
    // if you pass -1, it will be attached without a bone, i.e. placed relative
    // to the parent's base position instead
    void Attach(Object3D* toParent, short boneIndex);
    // attaches this object to the specified object at the specified bone.
    // if the bone does not exist (or you pass NULL), it will be attached without
    // a bone, i.e. placed relative to the parent's base position instead
    void Attach(Object3D* toParent, const char* boneName);

    // if this is a child, remove it from the parent's linked list of children
    // if this is a parent, remove everything from the list
    void Detach();
    void DrawChildren(Object3D* topLevelObject);

    bool IsChild() const;
    bool IsParent() const;

    int GetFlag(int mask) const;
    void EnableFlag(int mask);
    void DisableFlag(int mask);
    int GetFlags() const;

    void SetNoTextureTimer(int timer);

    void SetTexturePaletteOffset(unsigned int offset);
    unsigned int GetTexturePaletteOffset() const;

    // The alpha scale factor is only used for rendering, and is not 
    // passed to an object's children
    void SetAlphaScaleFactor(fix32_t value);
    fix32_t GetAlphaScaleFactor() const;

    void SetField9e(unsigned char value);
    unsigned char GetField9e() const;

    void TrackBone(TrackedBoneMatrix* trackingEntry);
    void UntrackBoneByIndex(int idx);
    void UntrackAllBones();
    TrackedBoneMatrix* GetTrackedBoneMatrix(int idx);

    void EnableBoneTracking();
    Vector3fix GetPointInFront(fix32_t distance) const;
};