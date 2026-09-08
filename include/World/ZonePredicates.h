#pragma once

// Zone-ID range checks used to classify a zone before its map archive is
// loaded. All three are pure range tests over the same block of IDs.

// usa: func_0201b588
bool IsGrottoZone(int zoneID);

// usa: func_0201b5b0
bool IsMainGrottoFloorZone(int zoneID);

// usa: func_0201b5d8
bool IsGrottoBossFloorZone(int zoneID);
