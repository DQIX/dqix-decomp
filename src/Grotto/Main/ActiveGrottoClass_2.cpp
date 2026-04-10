#include "Grotto/Main/ActiveGrottoData.h"
#include "Combat/Main/BattleList.h"
#include <globaldefs.h>

#ifdef jpn
    #define func_020a40e8 func_020a5f00
    #define func_02032544 func_0203207c
#endif

extern "C"
{
    // Just (vector)-memsets to zero
    void func_020a40e8(ActiveGrottoClass::UniversalData*);

    // probably an allocation - see FloorMap.cpp
    void* func_02032544(void* allocator, unsigned int size);
}

// USA: func_0209033c
// JPN: func_02090c5c
void ActiveGrottoClass::Clear()
{
    pGenerator = NULL;
    func_020a40e8(&universal);
    floorMap.Clear();
    floorWidth = 16;
    floorHeight = 16;
    unknown_260[0] = 0;
    unknown_260[1] = 0;
    unknown_260[2] = 0;
    unknown_264[0] = 0;
    unknown_264[1] = 0;
    unknown_264[2] = 0;
    unknown_264[3] = 0;
    unknown_274 = 0;
    unknown_276 = 0;
}

// USA: func_02090394
// JPN: func_02090cb4
void ActiveGrottoClass::ClearGenerator(bool keepFloormap)
{
    pGenerator = NULL;
    if (!keepFloormap)
        floorMap.Clear();
}

// USA: func_020903b4
// JPN: func_02090cd4
void ActiveGrottoClass::BlankFunction2() const
{
}

// USA: func_020903b8
// JPN: func_02090cd8
void ActiveGrottoClass::AllocateGenerator(void* allocator, bool skipAllocMapBuffers)
{
    if (pGenerator == NULL)
    {
        pGenerator = (FloorMapGenerator*)func_02032544(allocator, sizeof(FloorMapGenerator));
        pGenerator->Initialize();
    }

    if (!skipAllocMapBuffers)
        floorMap.AllocateBuffers(allocator);
}

// USA: func_02090400
// JPN: func_02090d20
int ActiveGrottoClass::GetMapDimensionFromRange(int minimum, int maximum, int floor) const
{
    GetActiveGrottoSeed();
    if (minimum == maximum)
        return minimum;
    
    return minimum + (GetActiveGrottoSeed() + floor) % (maximum - minimum + 1);
}