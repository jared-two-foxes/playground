
#include "Grid.h"


// Creates a uniform N*N grid with dx spacing.
Grid Grid_Create(uint32_t n, float dx) 
{   
    Grid grid;
    grid.N = n;
    grid.vertices = new GLfloat[4*n*n];

    // Prepare Vertex Data
    int lIndex = 0;
    float lWidth = (n-1)*dx;
    float lHalfWidth = lWidth / 2;
    for (int i = 0; i < n; ++i) 
    {
        float x = i*dx - lHalfWidth;
        for (int j = 0; j < n; ++j) 
        {
            float z = j*dx - lHalfWidth;
            grid.vertices[lIndex+0] = x;
            grid.vertices[lIndex+1] = 0;
            grid.vertices[lIndex+2] = z;
            grid.vertices[lIndex+3] = 1.f;
            lIndex+=4;
        }
    } 

    return grid;
}

void Grid_MutateY(Grid* pGrid, std::function<float (float, float)> fn)
{
    uint32_t lIndex = 0;
    for (int i = 0; i < pGrid->N; ++i) 
    {
        for (int j = 0; j < pGrid->N; ++j) 
        {
            float x = pGrid->vertices[lIndex+0];
            float z = pGrid->vertices[lIndex+2];
            float y = fn(x,z);
            pGrid->vertices[lIndex+1] = y;
            lIndex += 4;
        }
    }
}

MeshCache InitializeMeshCache(const Grid* pGrid) 
{
    uint32_t lVertexCount = pGrid->N*pGrid->N;
    float* lNormals = new float[3*lVertexCount];
    //float* lUVs = new float[2*lVertexCount];

    // Prepare Vertex Data
    int lIndex = 0;
    for (int i = 0, n = pGrid->N; i < n; ++i) 
    {
        for (int j = 0; j < n; ++j)  
        {
            // Populate the Normals.
            lNormals[lIndex] = 0;
            lNormals[lIndex+1] = 1.f;
            lNormals[lIndex+2] = 0;

            //uvs[];

            lIndex += 3;
        }
    } 

    // Build the Index Data
    // Theres 2 triangles per square and theres going to be (row-1)*(row-1) squares
    uint32_t lTriangleCount = 2*(pGrid->N-1)*(pGrid->N-1);
    GLuint* lIndices = new GLuint[3*lTriangleCount];

    lIndex=0;
    for (int i = 0, n = pGrid->N-1; i < n; ++i)
    {
        for (int j = 0; j < n; ++j) 
        {
            int row1 = j * (n+1);
            int row2 = (j+1) * (n+1);  

            // triangle 1
            lIndices[lIndex++] = row1+i;
            lIndices[lIndex++] = row1+i+1;
            lIndices[lIndex++] = row2+i+1; 

            // triangle 2
            lIndices[lIndex++] = row1+i;
            lIndices[lIndex++] = row2+i+1;
            lIndices[lIndex++] = row2+i; 
        }
    }

    MeshCache cache;
    cache.HasNormal = true;
    cache.HasUV = false;
    cache.AllByControlPoints = false;
    cache.SubMeshes.push_back({0, lTriangleCount});

    glGenBuffers( 1, &cache.VBONames[MeshCache::VERTEX_VBO] );
    glBindBuffer( GL_ARRAY_BUFFER, cache.VBONames[MeshCache::VERTEX_VBO] );
    glBufferData( GL_ARRAY_BUFFER, 4*lVertexCount*sizeof(GLfloat), &pGrid->vertices[0], GL_STATIC_DRAW );
    
    glGenBuffers( 1, &cache.VBONames[MeshCache::NORMAL_VBO] );
    glBindBuffer( GL_ARRAY_BUFFER, cache.VBONames[MeshCache::NORMAL_VBO] );
    glBufferData( GL_ARRAY_BUFFER, 3*lVertexCount*sizeof(GLfloat), &lNormals[0], GL_STATIC_DRAW );
    
    glGenBuffers( 1, &cache.VBONames[MeshCache::INDEX_VBO] );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cache.VBONames[MeshCache::INDEX_VBO] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 3*lTriangleCount*sizeof(GLuint), &lIndices[0], GL_STATIC_DRAW );

    delete [] lIndices;
    delete [] lNormals;

    return cache;
}