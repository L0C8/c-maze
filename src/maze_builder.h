#pragma once

// Fills map with a generated maze: 0 for empty floor, 1 for wall.
// width and height map to X (columns) and Z (rows) respectively.
void GenerateMaze(int width, int height, int *map);
