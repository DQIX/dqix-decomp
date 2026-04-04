#pragma once

struct GrottoTilePoint
{
    char x;
    char y;
};

// sizeof(FloorMap) == 140. (Look at buffer size in
// func_ov017_021b4a88 to confirm this).
class FloorMap
{
public:
    // This is held externally, though seems to be held right after the 
    // end of the generator.
    unsigned char* pMapData; 
    // Also held externally, seemingly right after the map data.
    unsigned char* pMapAdjacencyData;

    int width;
    int height;
    GrottoTilePoint stairsUp;
    GrottoTilePoint stairsDown;

    char unknown[104];

    char unknownBuffer[10];
    char padding[2];
    int unknownInt; // maybe number of chests?

public:
    bool Initialize(int width, int height);
    void InitBuffers();

    bool ComputeAdjacencyData();
    void ComputeSingleAdjacencyDataPoint(int x, int y, unsigned char* outData);

    void WriteTile(int x, int y, unsigned char value);
    int GetTile(int x, int y) const;

    void WriteRectangle(int left, int top, int width, int height, unsigned char value);

    int GetAdjacencyBits(int x, int y) const;

    void AllocateBuffers(void* allocator);

    // These do the exact same thing (v2 calls v1).
    // Based on where Clear2 gets called, it might be a destructor (albeit a pointless one)
    void Clear();
    void Clear2();
};