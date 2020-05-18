
#include "World.h"
#include "Fbx.h"
#include "Intersect.h"


//@todo: Convert these from pointers to pod arrays [jared.watt]
std::vector<LightCache* > Lights;
std::vector<MaterialCache* > Materials;
std::vector<MeshCache* > Meshes;
std::vector<TextureCache* > Textures;

std::vector<WorldObject > Objects;
std::vector<WorldActor  > Instances;


NsAxisAlignedBox WorldObject_GetBounds(WorldObject* pObject)
{
    // @todo: [jared.watt 22/04/20]
    //  This should iterate a list of primitives and generate the bounds based
    //  on this.  This would solve the problem of dynamic Aabb during animations.
    //
    return pObject->Aabb;
}

NsAxisAlignedBox WorldActor_GetBounds(WorldActor* aActor)
{
    FbxMatrix lLocalTransform(aActor->WorldPosition.Position, 
        aActor->WorldPosition.Rotation, FbxVector4(1,1,1));
    return Transform(aActor->Object->Aabb, lLocalTransform);
}

void Fbx_ImportNodeRecursive(WorldObject* pObject, FbxNode* pNode) 
{
    // @todo: [jared.watt, 23/04/20]
    //   Handle the positioning of components based upon node position.  
    // GetGlobalPosition(pNode, nullptr)
    
    // Lets pull any FbxNodeAttributes that were interested in.
    // @todo: [jared.watt, 22/04/20]
    //   Iterate all the NodeAttributes rather than assume 
    //   the only thing that we care about is in the default node.
    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
    if (lNodeAttribute)
    {
        //@todo:  Interestingly an FbxNodeAttribute can have its own
        //        offset from the node which isnt accounted for. [jared.watt]

        // Bake mesh as VBO(vertex buffer object) into GPU.
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            // We store the RenderPrimitive object on the Attribute so 
            // we're going to grab that.
            FbxMesh* lMesh = pNode->GetMesh();

            // Pull the Primitive object.
            MeshCache* lMeshCache = static_cast<MeshCache*>(lMesh->GetUserDataPtr());
            if (lMeshCache)
            {
                // @todo: Check if this object already exists in the worlds
                //        list [jared.watt]
                Meshes.push_back(lMeshCache);

                // For each mesh we can calculate its overall bounds and expand
                // recorded object bounds where needed.
                NsAxisAlignedBox aabb = Fbx_CalculateBounds(lMesh);
                pObject->Aabb = Expand(pObject->Aabb, aabb);
                
                // Finally add it to the objects Meshes.
                pObject->Meshes.push_back(lMeshCache);
            }
        }
        // Bake light properties.
        /*else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
        {
    
        }*/
    }

    // And recurse the child nodes.
    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        Fbx_ImportNodeRecursive(pObject, pNode->GetChild(lChildIndex));
    }
}

WorldObject* World_ImportFbxScene( const std::string& pName, FbxScene* pScene )
{
    // Create the Object to store the data from this scene.
    Objects.push_back(WorldObject());
    WorldObject& lObject = Objects.back();
    lObject.FileName = pName;

    // First we can grab all the textures and materials that are used
    // in this scene so lets go ahead and do that. 
    
    // First lets grab all the textures in the scene.
    FbxArray<FbxTexture* > textures;
    pScene->FillTextureArray(textures);
    for (int i = 0, n = textures.Size(); i < n; ++i) 
    {
        // During the import we bake all the Render Primitives
        // into the objects by setting them as user data pointers
        // so grab it and jam it into the world's lists.
        FbxTexture* lFbxTexture = textures[i];
        TextureCache* lCache = (TextureCache*)lFbxTexture->GetUserDataPtr();
        //assert(lCache);
        if (lCache)
        {
            Textures.push_back(lCache);
            lObject.Textures.push_back(lCache);
        }
    }

    // And secondly all the Materials in the scene.
    FbxArray<FbxSurfaceMaterial* > materials;
    pScene->FillMaterialArray(materials);
    for (int i = 0, n = materials.Size(); i < n; ++i) 
    {
        FbxSurfaceMaterial* lFbxSurfaceMaterial = materials[i];
        MaterialCache* lCache = (MaterialCache*)lFbxSurfaceMaterial->GetUserDataPtr();
        //assert(lCache);
        if (lCache) 
        {
            Materials.push_back(lCache);
            lObject.Materials.push_back(lCache);
        }
    }

    // Okay awesome, now were going to have to iterate the scene 
    // and pull out all of the FbxNodeAttributes and push them into
    // the world.
    FbxNode* lRootNode = pScene->GetRootNode();

    // The root node isn't going to have anything attached to it so
    // were going to start with its children.
    Fbx_ImportNodeRecursive(&lObject, lRootNode);

    return &lObject;
}

/*WorldObject* World_GetObjectByName(const std::string& name)
{
    for (auto& o : Objects) 
    {
        if (o.Name == name) 
        {
            return &o;
        }
    }
    return nullptr;
}*/

WorldActor* 
World_RayCast(/*World*/ NsRay& aRay)
{
    //@todo: [jared.watt, 22/04/20]
    //  We need to sort these objects by distance from the ray origin 
    //  so that the returned object is the closest rather than the 
    //  first.

    FbxVector4 lOut;
    for (auto& lActor : Instances) {        
        NsAxisAlignedBox lAabb = WorldActor_GetBounds(&lActor);
        if (Intersect(aRay, lAabb, &lOut)) {
            return &lActor;
        }
    }
    return nullptr;
}

WorldActor* 
World_AddInstance(FbxVector4 pPosition, WorldObject* pObject, const std::string& aName) 
{
    // Add it to an instance.
    Instances.push_back(WorldActor());
    WorldActor& instance = Instances.back();
    instance.WorldPosition.Position = pPosition;
    instance.WorldPosition.Rotation = FbxQuaternion();
    instance.Object = pObject;
    instance.Name = aName;
    return &instance;
}

void World_Draw()
{
    for (auto& lInstance : Instances) 
    {
        for (auto& lMesh : lInstance.Object->Meshes) 
        {
            DrawMeshCache(lMesh, lInstance.WorldPosition.Position, 
                lInstance.WorldPosition.Rotation);
        }
    }
}