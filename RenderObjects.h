#ifndef RENDEROBJECTS_H
#define RENDEROBJECTS_H

#include "Common.h"



void DrawNull(FbxAMatrix& pGlobalPosition);
void DrawMarker(FbxAMatrix& pGlobalPosition);


struct SubMesh 
{
    uint16_t IndexOffset;
    uint16_t TriangleCount;
};

// Cache for FBX Mesh
// @todo: 
//  - Destructor
//      - Delete vbonames
//      - Delete submeshes
struct MeshCache 
{
    enum 
    {
        VERTEX_VBO,
        NORMAL_VBO,
        UV_VBO,
        INDEX_VBO,
        VBO_COUNT
    };
    
    MeshCache();

    GLuint VBONames[VBO_COUNT]; 
    std::vector<SubMesh > SubMeshes;
    bool HasNormal;
    bool HasUV;
    bool AllByControlPoints; // Save data in VBO by control point or by polygon vertex 
};

// Draw the vertices of a mesh.
void DrawMeshCache( const MeshCache* pNode, FbxAMatrix& pGlobalPosition);


struct ColorChannel
{
    ColorChannel();

    GLuint TextureName;
    GLfloat Colour[4];
};

// Cache for FBX material
struct MaterialCache
{
    MaterialCache();
    
    ColorChannel Emissive;
    ColorChannel Ambient;
    ColorChannel Diffuse;
    ColorChannel Specular;
    GLfloat Shinness;
};

void SetCurrentMaterialCache( const MaterialCache* pMaterialCache );
bool HasTexture( const MaterialCache* pMaterialCache );
void SetDefaultMaterialCache();

struct LightCache 
{
    enum Type
    {
        Point, 
        Directional, 
    };
    
    LightCache();

    Type Type;
    GLfloat Colour[3];
};

void SetLight(const LightCache* pLightCache);

#endif // RENDEROBJECTS_H