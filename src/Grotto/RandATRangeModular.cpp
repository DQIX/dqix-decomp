#include <globaldefs.h>
#include "std_library_functions.h"
#include "Grotto/RandATRangeModular.h"

// USA: func_020a3ec0
// JPN: func_020a5bfc
unsigned int RandATRangeModular(unsigned int minimum, unsigned int maximum)
{
    return minimum + ((unsigned int)rand() % (maximum - minimum + 1));
}