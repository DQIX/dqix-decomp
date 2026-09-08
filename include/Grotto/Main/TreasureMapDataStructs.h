#pragma once

#include "TreasureMapMetadata.h"
#include "DetailedTreasureMapData.h"

bool ExportDetailedTreasureMapData(const TreasureMapMetadata* from,
    DetailedTreasureMapData* to, bool computeLegacyStats,
    const unsigned char* legacyStatsData);
bool ExportTreasureMapMetadata(const DetailedTreasureMapData* from, TreasureMapMetadata* to);

// Includes the endpoints.
// Gets a random number from AT via the usual rand() % range + min trick.
// Doesn't perform any range checks: AT always advances.
// Looks like a more generic function, but this version is only used
// by grotto functions so we put it here.
unsigned int RandATRangeModular(unsigned int minimum, unsigned int maximum);