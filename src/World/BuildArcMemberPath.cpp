#include <globaldefs.h>
#include "std_library_functions.h"
#include "World/MapArchive.h"

extern char data_020ef1de[]; // "ARC:/%s"
extern char data_020ef1e6[]; // "."

// usa: func_02014d18
//
// Names a member inside an already-open archive: "ARC:/" plus the stem, with
// the extension replaced. A stem that carries no dot gets one appended, so the
// caller can pass either an authored name or a bare stem.
extern "C" ARM void BuildArcMemberPath(const char* stem, const char* extension, char* path)
{
    sprintf(path, data_020ef1de, stem);

    char* dot = strrchr(path, '.');
    if (dot)
    {
        strcpy(dot + 1, extension);
    }
    else
    {
        strcat(path, data_020ef1e6);
        strcat(path, extension);
    }
}
