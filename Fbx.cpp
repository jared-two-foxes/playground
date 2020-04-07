
#include "Fbx.h"
#include "targa.h"


namespace {
    const int TRIANGLE_VERTEX_COUNT = 3;

    const int POSITION_STRIDE = 4;  //< Four floats for every position.
    const int NORMAL_STRIDE = 3;    //< Three floats for every normal.
    const int UV_STRIDE = 2;        //< Two floats for every UV.

    const GLfloat DEFAULT_LIGHT_POSITION[] = {0.0f, 0.0f, 0.0f, 1.0f};
    const GLfloat DEFAULT_DIRECTION_LIGHT_POSITION[] = {0.0f, 0.0f, 1.0f, 0.0f};
    const GLfloat DEFAULT_SPOT_LIGHT_DIRECTION[] = {0.0f, 0.0f, -1.0f};
    const GLfloat DEFAULT_LIGHT_COLOR[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat DEFAULT_LIGHT_SPOT_CUTOFF = 180.0f;
}

FbxManager* lSdkManager;
FbxImporter* lImporter;


void ConvertFbxToLinmath( const double* pDouble, GLfloat* pFloat, std::size_t n ) {
    for (int i =0; i<n; ++i ) {
        pFloat[i] = pDouble[i];
    }
}

void InitializeSdkObjects( FbxManager*& pManager ) 
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if( !pManager )
    {
        printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
	else 
    {
        printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());
    }

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());
}

FbxScene* CreateSceneObject(const char* name) 
{
    return FbxScene::Create(lSdkManager, name);
}

void DestroySdkObjects(FbxManager* pManager/*, bool pExitStatus*/)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if( pManager ) pManager->Destroy();
	//if( pExitStatus ) printf("Program Success!\n");
}

void InitFbx()
{
    // Create SDK manager.    
    InitializeSdkObjects(lSdkManager);
}


// Load a texture file (TGA only now) into GPU and return the texture object name
bool 
LoadTextureFromFile( const FbxString & pFilePath, unsigned int & pTextureObject )
{
    if (pFilePath.Right(3).Upper() == "TGA")
    {
        tga_image lTGAImage;

        if (tga_read(&lTGAImage, pFilePath.Buffer()) == TGA_NOERR)
        {
            // Make sure the image is left to right
            if (tga_is_right_to_left(&lTGAImage)) 
            {
                tga_flip_horiz(&lTGAImage);
            }

            // Make sure the image is bottom to top
            if (tga_is_top_to_bottom(&lTGAImage))
            {    
                tga_flip_vert(&lTGAImage);
            }

            // Make the image BGR 24
            tga_convert_depth(&lTGAImage, 24);

            // Transfer the texture date into GPU
            glGenTextures(1, &pTextureObject);
            glBindTexture(GL_TEXTURE_2D, pTextureObject);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTexImage2D(GL_TEXTURE_2D, 0, 3, lTGAImage.width, lTGAImage.height, 0, GL_BGR,
                GL_UNSIGNED_BYTE, lTGAImage.image_data);
            glBindTexture(GL_TEXTURE_2D, 0);

            tga_free_buffers(&lTGAImage);

            return true;
        }
    }

    return false;
}


// Bake node attributes and materials under this node recursively.
// Currently only mesh, light and material.
void LoadCacheRecursive(FbxNode * pNode, bool pSupportVBO)
{
    // Bake material and hook as user data.
    const int lMaterialCount = pNode->GetMaterialCount();
    for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
    {
        FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lMaterialIndex);
        if (lMaterial && !lMaterial->GetUserDataPtr())
        {
            MaterialCache* lMaterialCache = InitializeMaterialCache(lMaterial);
            if (lMaterialCache) 
            {
                lMaterial->SetUserDataPtr(lMaterialCache);
            }
        }
    }

    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
    if (lNodeAttribute)
    {
        // Bake mesh as VBO(vertex buffer object) into GPU.
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* lMesh = pNode->GetMesh();
            if (pSupportVBO && lMesh && !lMesh->GetUserDataPtr())
            {
                MeshCache* lMeshCache = InitializeMeshCache(lMesh); 
                if ( lMeshCache ) 
                {
                    lMesh->SetUserDataPtr(lMeshCache);
                }
            }
        }
        // Bake light properties.
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
        {
            FbxLight * lLight = pNode->GetLight();
            if (lLight && !lLight->GetUserDataPtr())
            {
                LightCache* lLightCache = InitializeLightCache(lLight);
                if (lLightCache) 
                {
                    lLight->SetUserDataPtr(lLightCache);
                }
            }
        }
    }

    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        LoadCacheRecursive(pNode->GetChild(lChildIndex), pSupportVBO);
    }
}

void LoadCacheRecursive(FbxScene * pScene, bool pSupportVBO)
{
    // Load the textures into GPU, only for file texture now
    const int lTextureCount = pScene->GetTextureCount();
    for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
    {
        FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
        FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
        if (lFileTexture && !lFileTexture->GetUserDataPtr())
        {
            // Try to load the texture from absolute path
            const FbxString lFileName = lFileTexture->GetFileName();
            
            // Only TGA textures are supported now.
            if (lFileName.Right(3).Upper() != "TGA")
            {
                FBXSDK_printf("Only TGA textures are supported now: %s\n", lFileName.Buffer());
                continue;
            }

            GLuint lTextureObject = 0;
            bool lStatus = LoadTextureFromFile(lFileName, lTextureObject);

            // const FbxString lAbsFbxFileName = FbxPathUtils::Resolve(pFbxFileName);
            // const FbxString lAbsFolderName = FbxPathUtils::GetFolderName(lAbsFbxFileName);
            // if (!lStatus)
            // {
            //     // Load texture from relative file name (relative to FBX file)
            //     const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lFileTexture->GetRelativeFileName());
            //     lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
            // }

            // if (!lStatus)
            // {
            //     // Load texture from file name only (relative to FBX file)
            //     const FbxString lTextureFileName = FbxPathUtils::GetFileName(lFileName);
            //     const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lTextureFileName);
            //     lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
            // }

            //if (!lStatus)
            {
                FBXSDK_printf("Failed to load texture file: %s\n", lFileName.Buffer());
                continue;
            }

            if (lStatus)
            {
                GLuint * lTextureName = new GLuint(lTextureObject);
                lFileTexture->SetUserDataPtr(lTextureName);
            }
        }
    }

    LoadCacheRecursive(pScene->GetRootNode(), pSupportVBO);
}


bool 
ImportScene( const char* filename, FbxScene*& lScene ) 
{
    // Create the importer.
    int lFileFormat = -1;
    lImporter = FbxImporter::Create( lSdkManager, "" );
    if (!lSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(filename, lFileFormat) )
    {
        // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
        lFileFormat = lSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );
    }

    // Use the first argument as the filename for the importer.
    if (!lImporter->Initialize(filename, lFileFormat, lSdkManager->GetIOSettings())) 
    {
        printf("Call to FbxImporter::Initialize() failed.\n" );
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }
    
    bool lResult = lImporter->Import( lScene );
    if ( lResult )
    {
        // Check the scene integrity!
        FbxStatus status;
        FbxArray<FbxString* > details;
        FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(lScene), &status, &details);
        bool lNotify = (!sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData) && details.GetCount() > 0) || (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
        if (lNotify)
        {
            printf("\n");
            printf("********************************************************************************\n");
            if (details.GetCount())
            {
                printf("Scene integrity verification failed with the following errors:\n");

                for (int i = 0; i < details.GetCount(); i++)
                    printf("   %s\n", details[i]->Buffer());

                FbxArrayDelete<FbxString*>(details);
            }

            if (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess)
            {
                printf("\n");
                printf("WARNING:\n");
                printf("   The importer was able to read the file but with errors.\n");
                printf("   Loaded scene may be incomplete.\n\n");
                printf("   Last error message:'%s'\n", lImporter->GetStatus().GetErrorString());
            }
            
            printf("********************************************************************************\n");
            printf("\n");
        }

        // Convert Axis System to what is used in this example, if needed
        FbxAxisSystem SceneAxisSystem = lScene->GetGlobalSettings().GetAxisSystem();
        FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
        if (SceneAxisSystem != OurAxisSystem)
        {
            OurAxisSystem.DeepConvertScene(lScene);
        }

        // Convert Unit System to what is used in this example, if needed
        FbxSystemUnit SceneSystemUnit = lScene->GetGlobalSettings().GetSystemUnit();
        if (SceneSystemUnit.GetScaleFactor() != 1.0)
        {
            //The unit in this example is centimeter.
            FbxSystemUnit::cm.ConvertScene(lScene);
        }

        // Convert mesh, NURBS and patch into triangle mesh
        FbxGeometryConverter lGeomConverter(lSdkManager);
        try {
            lGeomConverter.Triangulate(lScene, /*replace*/true);
        }
        catch (std::runtime_error) {
            printf("Scene integrity verification failed.\n");
            return false;
        }

        // Bake the scene for one frame
        LoadCacheRecursive(lScene, /*mSupportVBO*/true);
    }

    // Destroy the importer to release the file.
    lImporter->Destroy();
    lImporter = NULL;

    return lResult;
}


// Get the global position of the node for the current pose.
// If the specified node is not part of the pose or no pose is specified, get its
// global position at the current time.
FbxAMatrix 
GetGlobalPosition(
    FbxNode* pNode,
    FbxAMatrix* pParentGlobalPosition)
{
    FbxAMatrix lGlobalPosition;

    // There is no pose entry for that node, get the current global position instead.

    // Ideally this would use parent global position and local position to compute the global position.
    // Unfortunately the equation 
    //    lGlobalPosition = pParentGlobalPosition * lLocalPosition
    // does not hold when inheritance type is other than "Parent" (RSrs).
    // To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
    lGlobalPosition = pNode->EvaluateGlobalTransform(/*pTime*/);
    
    return lGlobalPosition;
}


// Print the required number of tabs.
// Tabs caracter ("\t") counter
int numTabs = 0;
void PrintTabs() {
    for (int i = 0; i < numTabs; i++ ) {
        printf("\t");
    }
}

// Return a string-based representation based on the attribute type.
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) { 
    switch(type) { 
        case FbxNodeAttribute::eUnknown: return "unidentified"; 
        case FbxNodeAttribute::eNull: return "null"; 
        case FbxNodeAttribute::eMarker: return "marker"; 
        case FbxNodeAttribute::eSkeleton: return "skeleton"; 
        case FbxNodeAttribute::eMesh: return "mesh"; 
        case FbxNodeAttribute::eNurbs: return "nurbs"; 
        case FbxNodeAttribute::ePatch: return "patch"; 
        case FbxNodeAttribute::eCamera: return "camera"; 
        case FbxNodeAttribute::eCameraStereo: return "stereo"; 
        case FbxNodeAttribute::eCameraSwitcher: return "camera switcher"; 
        case FbxNodeAttribute::eLight: return "light"; 
        case FbxNodeAttribute::eOpticalReference: return "optical reference"; 
        case FbxNodeAttribute::eOpticalMarker: return "marker"; 
        case FbxNodeAttribute::eNurbsCurve: return "nurbs curve"; 
        case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface"; 
        case FbxNodeAttribute::eBoundary: return "boundary"; 
        case FbxNodeAttribute::eNurbsSurface: return "nurbs surface"; 
        case FbxNodeAttribute::eShape: return "shape"; 
        case FbxNodeAttribute::eLODGroup: return "lodgroup"; 
        case FbxNodeAttribute::eSubDiv: return "subdiv"; 
        default: return "unknown"; 
    } 
}

// Print an attribute.
void PrintAttribute(FbxNodeAttribute* pAttribute) {
    if(!pAttribute) return;
 
    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();
    PrintTabs();
    // Note: to retrieve the character array of a FbxString, use its Buffer() method.
    printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

void PrintNode(FbxNode* pNode) 
{
    PrintTabs();
    const char* nodeName = pNode->GetName();
    FbxDouble3 translation = pNode->LclTranslation.Get();
    FbxDouble3 rotation = pNode->LclRotation.Get();
    FbxDouble3 scaling = pNode->LclScaling.Get();

    // Pritn the contents of the node.
    printf("<node name='%s' translation='(%f %f %f)' rotation='(%f %f %f)' scaling='(%f, %f, %f)'>\n", 
        nodeName, 
        translation[0], translation[1], translation[2],
        rotation[0], rotation[1], rotation[2],
        scaling[0], scaling[1], scaling[2]
        );
    numTabs++;

    // Print the nodes attributes.
    for( int i = 0; i < pNode->GetNodeAttributeCount(); i++) 
    {
        PrintAttribute(pNode->GetNodeAttributeByIndex(i));
    }

    // Recursively print the children
    for (int j = 0; j < pNode->GetChildCount(); j++) 
    {
        PrintNode(pNode->GetChild(j));
    }

    numTabs--;
    PrintTabs();
    printf("</node>\n");
}

void PrintScene(FbxScene* lScene) 
{
    // Print the nodes of the scene and their attributes recursively.
    // Note that we are not printing the root node because it should
    // not contain any attributes.
    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode) {
        for (int i = 0; i < lRootNode->GetChildCount(); i++ ) {
            PrintNode(lRootNode->GetChild(i));
        }
    }
}

MeshCache* InitializeMeshCache( const FbxMesh* pMesh ) 
{
    if (!pMesh->GetNode()) 
    {
        return nullptr;
    }

    MeshCache* pMeshCache = new MeshCache();

    const int lPolygonCount = pMesh->GetPolygonCount();

    // Count the polygon count of each material
    FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
    FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
    if (pMesh->GetElementMaterial())
    {
        lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
        lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            assert(lMaterialIndice->GetCount() == lPolygonCount);
            if (lMaterialIndice->GetCount() == lPolygonCount)
            {
                // Count the faces of each material
                for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                {
                    const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
                    if (pMeshCache->SubMeshes.size() < lMaterialIndex + 1)
                    {
                        pMeshCache->SubMeshes.resize(lMaterialIndex + 1);
                    }
                    
                    pMeshCache->SubMeshes[lMaterialIndex].TriangleCount += 1;
                }

                // Record the offset (how many vertex)
                const int lMaterialCount = pMeshCache->SubMeshes.size();
                int lOffset = 0;
                for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                {
                    pMeshCache->SubMeshes[lIndex].IndexOffset = lOffset;
                    lOffset += pMeshCache->SubMeshes[lIndex].TriangleCount * 3;
                    // This will be used as counter in the following procedures, reset to zero
                    pMeshCache->SubMeshes[lIndex].TriangleCount = 0;
                }
                assert(lOffset == lPolygonCount * 3);
            }
        }
    }

    // All faces will use the same material.
    if (pMeshCache->SubMeshes.size() == 0)
    {
        pMeshCache->SubMeshes.resize(1);
    }

    // Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    pMeshCache->HasNormal = pMesh->GetElementNormalCount() > 0;
    pMeshCache->HasUV = pMesh->GetElementUVCount() > 0;
    FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
    if (pMeshCache->HasNormal)
    {
        lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        if (lNormalMappingMode == FbxGeometryElement::eNone)
        {
            pMeshCache->HasNormal = false;
        }
        if (pMeshCache->HasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
        {
            pMeshCache->AllByControlPoints = false;
        }
    }
    if (pMeshCache->HasUV)
    {
        lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        if (lUVMappingMode == FbxGeometryElement::eNone)
        {
            pMeshCache->HasUV = false;
        }
        if (pMeshCache->HasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
        {
            pMeshCache->AllByControlPoints = false;
        }
    }

    // Allocate the array memory, by control point or by polygon vertex.
    int lPolygonVertexCount = pMesh->GetControlPointsCount();
    if (!pMeshCache->AllByControlPoints)
    {
        lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
    }
    float * lVertices = new float[lPolygonVertexCount * POSITION_STRIDE];
    unsigned int * lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    float * lNormals = NULL;
    if (pMeshCache->HasNormal)
    {
        lNormals = new float[lPolygonVertexCount * NORMAL_STRIDE];
    }
    float * lUVs = NULL;
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    const char * lUVName = NULL;
    if (pMeshCache->HasUV && lUVNames.GetCount())
    {
        lUVs = new float[lPolygonVertexCount * UV_STRIDE];
        lUVName = lUVNames[0];
    }

    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector2 lCurrentUV;
    if (pMeshCache->AllByControlPoints)
    {
        const FbxGeometryElementNormal * lNormalElement = NULL;
        const FbxGeometryElementUV * lUVElement = NULL;
        if (pMeshCache->HasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        if (pMeshCache->HasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
        {
            // Save the vertex position.
            lCurrentVertex = lControlPoints[lIndex];
            lVertices[lIndex * POSITION_STRIDE] = static_cast<float>(lCurrentVertex[0]);
            lVertices[lIndex * POSITION_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
            lVertices[lIndex * POSITION_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
            lVertices[lIndex * POSITION_STRIDE + 3] = 1;

            // Save the normal.
            if (pMeshCache->HasNormal)
            {
                int lNormalIndex = lIndex;
                if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                lNormals[lIndex * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
                lNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
                lNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
            }

            // Save the UV.
            if (pMeshCache->HasUV)
            {
                int lUVIndex = lIndex;
                if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
                lUVs[lIndex * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
                lUVs[lIndex * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
            }
        }
    }

    int lVertexCount = 0;
    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        // The material for current face.
        int lMaterialIndex = 0;
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
        }

        // Where should I save the vertex attribute index, according to the material
        const int lIndexOffset = pMeshCache->SubMeshes[lMaterialIndex].IndexOffset +
            pMeshCache->SubMeshes[lMaterialIndex].TriangleCount * 3;
        for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
        {
            const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			// If the lControlPointIndex is -1, we probably have a corrupted mesh data. At this point,
			// it is not guaranteed that the cache will work as expected.
			if (lControlPointIndex >= 0)
			{
				if (pMeshCache->AllByControlPoints)
				{
					lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
				}
				// Populate the array with vertex attribute, if by polygon vertex.
				else
				{
					lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);

					lCurrentVertex = lControlPoints[lControlPointIndex];
					lVertices[lVertexCount * POSITION_STRIDE] = static_cast<float>(lCurrentVertex[0]);
					lVertices[lVertexCount * POSITION_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
					lVertices[lVertexCount * POSITION_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
					lVertices[lVertexCount * POSITION_STRIDE + 3] = 1;

					if (pMeshCache->HasNormal)
					{
						pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
						lNormals[lVertexCount * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
						lNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
						lNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
					}

					if (pMeshCache->HasUV)
					{
						bool lUnmappedUV;
						pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
						lUVs[lVertexCount * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
						lUVs[lVertexCount * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
					}
				}
			}
            ++lVertexCount;
        }
        pMeshCache->SubMeshes[lMaterialIndex].TriangleCount += 1;
    }

    // Create VBOs
    glGenBuffers(MeshCache::VBO_COUNT, pMeshCache->VBONames);

    // Save vertex attributes into GPU
    glBindBuffer(GL_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::VERTEX_VBO]);
    glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * POSITION_STRIDE * sizeof(float), lVertices, GL_STATIC_DRAW);
    delete [] lVertices;

    if (pMeshCache->HasNormal)
    {
        glBindBuffer(GL_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::NORMAL_VBO]);
        glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * NORMAL_STRIDE * sizeof(float), lNormals, GL_STATIC_DRAW);
        delete [] lNormals;
    }
    
    if (pMeshCache->HasUV)
    {
        glBindBuffer(GL_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::UV_VBO]);
        glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * UV_STRIDE * sizeof(float), lUVs, GL_STATIC_DRAW);
        delete [] lUVs;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::INDEX_VBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lPolygonCount * TRIANGLE_VERTEX_COUNT * sizeof(unsigned int), lIndices, GL_STATIC_DRAW);
    delete [] lIndices;

    return pMeshCache;
}


// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
FbxDouble3 
GetMaterialProperty(
    const FbxSurfaceMaterial * pMaterial,
    const char * pPropertyName,
    const char * pFactorPropertyName,
    GLuint & pTextureName )
{
    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
    const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
    if (lProperty.IsValid() && lFactorProperty.IsValid())
    {
        lResult = lProperty.Get<FbxDouble3>();
        double lFactor = lFactorProperty.Get<FbxDouble>();
        if (lFactor != 1)
        {
            lResult[0] *= lFactor;
            lResult[1] *= lFactor;
            lResult[2] *= lFactor;
        }
    }

    if (lProperty.IsValid())
    {
        const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
        if (lTextureCount)
        {
            const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
            if (lTexture && lTexture->GetUserDataPtr())
            {
                pTextureName = *(static_cast<GLuint *>(lTexture->GetUserDataPtr()));
            }
        }
    }

    return lResult;
}


MaterialCache* 
InitializeMaterialCache( const FbxSurfaceMaterial * pMaterial ) 
{
    MaterialCache* cache = new MaterialCache();

    const FbxDouble3 lEmissive = GetMaterialProperty(pMaterial,
    FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, cache->Emissive.TextureName);
    cache->Emissive.Colour[0] = static_cast<GLfloat>(lEmissive[0]);
    cache->Emissive.Colour[1] = static_cast<GLfloat>(lEmissive[1]);
    cache->Emissive.Colour[2] = static_cast<GLfloat>(lEmissive[2]);

    const FbxDouble3 lAmbient = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, cache->Ambient.TextureName);
    cache->Ambient.Colour[0] = static_cast<GLfloat>(lAmbient[0]);
    cache->Ambient.Colour[1] = static_cast<GLfloat>(lAmbient[1]);
    cache->Ambient.Colour[2] = static_cast<GLfloat>(lAmbient[2]);

    const FbxDouble3 lDiffuse = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, cache->Diffuse.TextureName);
    cache->Diffuse.Colour[0] = static_cast<GLfloat>(lDiffuse[0]);
    cache->Diffuse.Colour[1] = static_cast<GLfloat>(lDiffuse[1]);
    cache->Diffuse.Colour[2] = static_cast<GLfloat>(lDiffuse[2]);

    const FbxDouble3 lSpecular = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, cache->Specular.TextureName);
    cache->Specular.Colour[0] = static_cast<GLfloat>(lSpecular[0]);
    cache->Specular.Colour[1] = static_cast<GLfloat>(lSpecular[1]);
    cache->Specular.Colour[2] = static_cast<GLfloat>(lSpecular[2]);

    FbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        double lShininess = lShininessProperty.Get<FbxDouble>();
        cache->Shinness = static_cast<GLfloat>(lShininess);
    }

    return cache;
}


// Find all the cameras under this node recursively.
void 
FillCameraArrayRecursive(FbxNode* pNode, FbxArray<FbxNode*>& pCameraArray)
{
    if (pNode)
    {
        if (pNode->GetNodeAttribute())
        {
            if (pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera)
            {
                pCameraArray.Add(pNode);
            }
        }

        const int lCount = pNode->GetChildCount();
        for (int i = 0; i < lCount; i++)
        {
            FillCameraArrayRecursive(pNode->GetChild(i), pCameraArray);
        }
    }
}


// Find all the cameras in this scene.
void 
FillCameraArray(FbxScene* pScene, FbxArray<FbxNode*>& pCameraArray)
{
    pCameraArray.Clear();

    FillCameraArrayRecursive(pScene->GetRootNode(), pCameraArray);
}


LightCache* 
InitializeLightCache(const FbxLight* pLight)
{
    LightCache* lLightCache = new LightCache();

    switch (pLight->LightType.Get())
    {
    case FbxLight::EType::eDirectional:
        lLightCache->Type = LightCache::Type::Directional;
        break;
    default:
        lLightCache->Type = LightCache::Type::Directional;
        break;
    }
    
    FbxPropertyT<FbxDouble3> lColorProperty = pLight->Color;
    FbxDouble3 lLightColor = lColorProperty.Get();
    lLightCache->Colour[0] = static_cast<float>(lLightColor[0]);
    lLightCache->Colour[1] = static_cast<float>(lLightColor[1]);
    lLightCache->Colour[2] = static_cast<float>(lLightColor[2]);

    return lLightCache;
}