#include "Grotto/Main/ActiveGrottoClass.h"
#include "Combat/Main/BattleList.h"
#include <globaldefs.h>

// USA: func_0209033c
// JPN: func_02090c5c
void ActiveGrottoClass::Clear()
{
    pGenerator_ = NULL;
    overallMapData_.Clear();
    floorMap_.Clear();
    floorWidth_ = 16;
    floorHeight_ = 16;
    unknown_260_[0] = 0;
    unknown_260_[1] = 0;
    unknown_260_[2] = 0;
    unknown_264_[0] = 0;
    unknown_264_[1] = 0;
    unknown_264_[2] = 0;
    unknown_264_[3] = 0;
    unknown_274_ = 0;
    unknown_276_ = 0;
}

// USA: func_02090394
// JPN: func_02090cb4
void ActiveGrottoClass::ClearGenerator(bool keepFloormap)
{
    pGenerator_ = NULL;
    if (!keepFloormap)
        floorMap_.Clear();
}

// USA: func_020903b4
// JPN: func_02090cd4
void ActiveGrottoClass::BlankFunction2() const
{
}

// USA: func_020903b8
// JPN: func_02090cd8
void ActiveGrottoClass::AllocateGenerator(SafeAllocator* allocator, bool skipAllocMapBuffers)
{
    if (pGenerator_ == NULL)
    {
        pGenerator_ = (FloorMapGenerator*)allocator->Allocate(sizeof(FloorMapGenerator));
        pGenerator_->Initialize();
    }

    if (!skipAllocMapBuffers)
        floorMap_.AllocateBuffers(allocator);
}

// USA: func_02090400
// JPN: func_02090d20
int ActiveGrottoClass::GetMapDimensionFromRange(int minimum, int maximum, int floor) const
{
    (void)GetActiveGrottoSeed();
    if (minimum == maximum)
        return minimum;
    
    return minimum + (GetActiveGrottoSeed() + floor) % (maximum - minimum + 1);
}