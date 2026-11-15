#include "Filesystem/GPCImplementationDetails.h"

char VersionedGPCSignature<1>::bytes[] = "GPC1";
char VersionedGPCSignature<0>::bytes[] = "GPC0";
char VersionedGPCSignature<2>::bytes[] = "GPC2";