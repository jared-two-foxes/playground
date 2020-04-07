#ifndef GRID_H
#define GRID_H

#include "Common.h"
#include "RenderObjects.h"

struct Grid
{
    uint32_t N;
    GLfloat* vertices;
};

// Creates a uniform N*N grid with dx spacing.
Grid CreateGrid(uint32_t n, float dx);

MeshCache InitializeMeshCache(const Grid* pGrid);

#endif // GRID_H