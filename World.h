#ifndef WORLD_H
#define WORLD_H

// @note: [jared.watt, 21/04/20] 
//   Move away from "common.h" include to a types which has the basic types
//   and the required dependencies for that since thats really what I'm after.
//   potentially just a rename and merge.
#include "Common.h"
// @note: [jared.watt, 21/04/20]
//   As above potentially move all the "intersect" style data structures
//   into a single header file rather than one per type.
#include "AxisAlignedBox.h"
#include "Ray.h"
// @note: [jared.watt, 21/04/20]s
//   Rename RenderPrimitives and implement as above for the other 2 type
//   files mentioned above.
#include "RenderObjects.h" 


// @note: [jared.watt, 21/04/20]
//   * Rename to NsWorldTemplate?
//   * This probably should also include the objects AABB although I'm unsure
//     how we handle animations with this since their likely going to be dynamic
//     unless its a list of primitives which gets updated during the animation
//     process.
struct WorldObject
{
    std::string                  FileName;
    std::vector<LightCache* >    Lights;
    std::vector<MaterialCache* > Materials;
    std::vector<MeshCache* >     Meshes;
    std::vector<TextureCache* >  Textures;
    NsAxisAlignedBox             Aabb;
};

NsAxisAlignedBox WorldObject_GetBounds(WorldObject* pObject);

struct Location 
{
    FbxVector4    Position;
    FbxQuaternion Rotation;
};

// @todo: [jared.watt, 22/04/20]
//  Rename NsObjectInstance
//  Move the Aabb from Object/template to Actor
//  rename "WorldPosition", poor name.
struct WorldActor 
// This is an inworld representation of an object.
{
    std::string  Name;
    Location     WorldPosition;
    WorldObject* Object;
};

NsAxisAlignedBox WorldActor_GetBounds(WorldActor* pObject);

WorldObject* World_ImportFbxScene(const std::string& name, /*const*/ FbxScene * pScene );

//WorldObject* World_GetObjectByName(const std::string& name);

WorldActor* World_RayCast(/*World*/ NsRay& aRay);

WorldActor* World_AddInstance(FbxVector4 pLocation, WorldObject* pObject, const std::string& Name);

void World_Draw();


#endif // WORLD_H