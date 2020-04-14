#ifndef WORLD_H
#define WORLD_H

#include "Common.h"
#include "RenderObjects.h" //<@todo: Rename RenderPrimitives? [jared.watt]

struct WorldObject
{
    std::string Name;
    std::vector<LightCache* > Lights;
    std::vector<MaterialCache* > Materials;
    std::vector<MeshCache* > Meshes;
    std::vector<TextureCache* > Textures;
};


WorldObject* World_ImportFbxScene(const std::string& name, /*const*/ FbxScene * pScene );

WorldObject* World_GetObjectByName(const std::string& name);

void World_AddInstance(FbxVector4 pLocation, WorldObject* pObject);

void World_Draw();

#endif // WORLD_H