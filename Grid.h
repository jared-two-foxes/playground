#ifndef GRID_H
#define GRID_H

#include "Common.h"
#include "RenderObjects.h"

#include <functional>

struct Grid
{
    uint32_t N;
    GLfloat* vertices;
};
// Creates a uniform N*N grid with dx spacing.

Grid Grid_Create(uint32_t n, float dx);
void Grid_MutateY(Grid* pGrid, std::function<float (float x, float z)> function);

MeshCache InitializeMeshCache(const Grid* pGrid);

#endif // GRID_H