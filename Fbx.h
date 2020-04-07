#ifndef FBX_HPP__
#define FBX_HPP__

#include "Common.h"
#include "RenderObjects.h"


void InitFbx();

FbxScene* CreateSceneObject(const char* name);

bool ImportScene(const char* filename, FbxScene*& pScene);

FbxAMatrix GetGlobalPosition(
    FbxNode* pNode,
    FbxAMatrix* pParentGlobalPosition = NULL);

void FillCameraArray(FbxScene* pScene, FbxArray<FbxNode*>& pCameraArray);

MeshCache* InitializeMeshCache( const FbxMesh* pMesh );
MaterialCache* InitializeMaterialCache( const FbxSurfaceMaterial * pMaterial );
LightCache* InitializeLightCache(const FbxLight * pLight);


#endif // FBX_HPP__