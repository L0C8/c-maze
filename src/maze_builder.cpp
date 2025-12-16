#include "maze_builder.h"

#include <algorithm>
#include <array>
#include <random>
#include <utility>
#include <vector>

namespace
{
int Index(int x, int y, int width)
{
    return y * width + x;
}
}

void GenerateMaze(int width, int height, int *map)
{
    if (!map || width < 2 || height < 2)
    {
        return;
    }

    // Clear interior to floor.
    const int total = width * height;
    for (int i = 0; i < total; ++i) map[i] = 0;

    // Build outer walls to form a simple square room.
    for (int x = 0; x < width; ++x)
    {
        map[Index(x, 0, width)] = 1;
        map[Index(x, height - 1, width)] = 1;
    }
    for (int y = 0; y < height; ++y)
    {
        map[Index(0, y, width)] = 1;
        map[Index(width - 1, y, width)] = 1;
    }
}
