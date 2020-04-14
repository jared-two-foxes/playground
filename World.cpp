
#include "World.h"
#include "Fbx.h"

struct Location 
{
    vec3 position;
    //quaternion rotation; //< ??
};

// This is an inworld representation of an object.
struct Instance 
{
    Location WorldPosition;
    WorldObject* Object;
};

//@todo: Convert these from pointers to pod arrays [jared.watt]
std::vector<LightCache* > Lights;
std::vector<MaterialCache* > Materials;
std::vector<MeshCache* > Meshes;
std::vector<TextureCache* > Textures;

std::vector<WorldObject > Objects;
std::vector<Instance  > Instances;


void Fbx_ImportNodeRecursive(WorldObject* pObject, FbxNode* pNode) 
{
    // Determine the position of the node in world space.
    FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, nullptr);
    // Grab the translation from the global position.
    FbxVector4 translation = lGlobalPosition.GetT();
    // Convert it to our vec3 type
    vec3 position;
    ConvertFbxToLinmath((const GLdouble*)lGlobalPosition, position, 3 );

    // Lets pull any FbxNodeAttributes that were interested in.
    // @todo: Iterate all the NodeAttributes rather than assume 
    //        the only thing that we care about is in the default node.
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

WorldObject* World_ImportFbxScene(const std::string& pName, FbxScene* pScene )
{
    // Create the Object to store the data from this scene.
    Objects.push_back(WorldObject());
    WorldObject& lObject = Objects.back();
    lObject.Name = pName;

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

WorldObject* World_GetObjectByName(const std::string& name)
{
    for (auto& o : Objects) 
    {
        if (o.Name == name) 
        {
            return &o;
        }
    }
    return nullptr;
}

void World_AddInstance(FbxVector4 pPosition, WorldObject* pObject) 
{
    // Add it to an instance.
    Instances.push_back(Instance());
    Instance& instance = Instances.back();
    ConvertFbxToLinmath( pPosition, instance.WorldPosition.position, 3 );
    instance.Object = pObject;
}

void World_Draw()
{
    for (auto& lInstance : Instances) 
    {
        for (auto& lMesh : lInstance.Object->Meshes) 
        {
            DrawMeshCache(lMesh, lInstance.WorldPosition.position);
        }
    }
}