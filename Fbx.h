#ifndef FBX_HPP__
#define FBX_HPP__

#include "Common.h"

void InitFbx();

FbxScene* CreateSceneObject(const char* name);

bool ImportScene(const char* filename, FbxScene*& pScene);

FbxAMatrix GetGlobalPosition(
    FbxNode* pNode,
    FbxAMatrix* pParentGlobalPosition = NULL);

void FillCameraArray(FbxScene* pScene, FbxArray<FbxNode*>& pCameraArray);



struct SubMesh 
{
    int IndexOffset;
    int TriangleCount;
};

// Cache for FBX Mesh
//todo: 
// Need to zero out the VBONames on initiation?
// Delete vbonames on destruction
// delete syubmeshes on destruction
struct MeshCache 
{
    enum {
        VERTEX_VBO,
        NORMAL_VBO,
        UV_VBO,
        INDEX_VBO,
        VBO_COUNT
    };
    
    MeshCache();

    GLuint VBONames[VBO_COUNT]; 
    FbxArray<SubMesh*> SubMeshes;
    bool HasNormal;
    bool HasUV;
    bool AllByControlPoints; // Save data in VBO by control point or by polydon vertex 
};

MeshCache* InitializeMeshCache( const FbxMesh* pMesh );
void BeginMeshCacheDraw(const MeshCache* pMeshCache);
void DrawMeshCache(const MeshCache* pMeshCache, int pMaterialIndex);
void EndMeshCacheDraw(const MeshCache* pMeshCache);

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

MaterialCache* InitializeMaterialCache( const FbxSurfaceMaterial * pMaterial );
void SetCurrentMaterialCache( const MaterialCache* pMaterialCache );
bool HasTexture( const MaterialCache* pMaterialCache );
void SetDefaultMaterialCache();


struct LightCache 
{
    LightCache();

    GLuint mLightIndex;
    FbxLight::EType mType;
    GLfloat mColorRed;
    GLfloat mColorGreen;
    GLfloat mColorBlue;
    GLfloat mConeAngle;
};

LightCache* InitializeLightCache(const FbxLight * pLight);
void SetLight(const LightCache* pLightCache);


#endif // FBX_HPP__