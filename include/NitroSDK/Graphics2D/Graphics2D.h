#pragma once

#include <std_library_functions.h>
#include "Common.h"

#include "AnimationBank.h"
#include "CellBank.h"
#include "Character.h"
#include "Palette.h"

Nitro2DInnerFileHeader* Nitro2D_GetInnerFileBySignature(void* fileData, uint32_t signature);