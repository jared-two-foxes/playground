#ifndef FBX_HPP__
#define FBX_HPP__

#include "Common.h"
#include "RenderObjects.h"
#include "AxisAlignedBox.h"

void InitFbx();

FbxScene* CreateSceneObject(const char* name);

bool ImportScene(const char* filename, FbxScene*& pScene);

FbxAMatrix GetGlobalPosition(
    FbxNode* pNode,
    FbxAMatrix* pParentGlobalPosition = NULL);

void FillCameraArray(FbxScene* pScene, FbxArray<FbxNode*>& pCameraArray);

TextureCache* InitializeTextureCache( const FbxTexture * pTexture );
MaterialCache* InitializeMaterialCache( const FbxSurfaceMaterial * pMaterial );
LightCache* InitializeLightCache(const FbxLight * pLight);
MeshCache* InitializeMeshCache( const FbxMesh* pMesh );

NsAxisAlignedBox Fbx_CalculateBounds( const FbxMesh* pMesh );

#endif // FBX_HPP__