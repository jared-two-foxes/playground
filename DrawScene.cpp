
#include "DrawScene.h"
#include "Fbx.h"
#include "RenderPipeline.h"

FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
FbxAMatrix GetGeometry(FbxNode* pNode);

void DrawNull(FbxAMatrix& pGlobalPosition);
void DrawMarker(FbxAMatrix& pGlobalPosition);
void DrawMesh(FbxNode* pNode, FbxAMatrix& pGlobalPosition);
void DrawLight(FbxNode* pNode, FbxAMatrix& pGlobalPosition);

void DrawNode(FbxNode* pNode, FbxAMatrix& pParentGlobalPosition, FbxAMatrix& pGlobalPosition);


void
IntializeEnvironment(const FbxColor & pAmbientLight)
{
    // glLightfv(GL_LIGHT0, GL_POSITION, DEFAULT_DIRECTION_LIGHT_POSITION);
    // glLightfv(GL_LIGHT0, GL_DIFFUSE, DEFAULT_LIGHT_COLOR);
    // glLightfv(GL_LIGHT0, GL_SPECULAR, DEFAULT_LIGHT_COLOR);
    // glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, DEFAULT_LIGHT_SPOT_CUTOFF);
    // glEnable(GL_LIGHT0);

    // // Set ambient light.
    // GLfloat lAmbientLight[] = {static_cast<GLfloat>(pAmbientLight[0]), static_cast<GLfloat>(pAmbientLight[1]),
    //     static_cast<GLfloat>(pAmbientLight[2]), 1.0f};
    // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lAmbientLight);
}


void 
InitializeLights(FbxScene* pScene)
{
    // Set ambient light. Turn on light0 and set its attributes to default (white directional light in Z axis).
    // If the scene contains at least one light, the attributes of light0 will be overridden.
    IntializeEnvironment(pScene->GetGlobalSettings().GetAmbientColor());

    // Setting the lights before drawing the whole scene
	const int lLightCount = pScene->GetSrcObjectCount<FbxLight>();
    for (int lLightIndex = 0; lLightIndex < lLightCount; ++lLightIndex)
    {
        FbxLight * lLight = pScene->GetSrcObject<FbxLight>(lLightIndex);
        FbxNode * lNode = lLight->GetNode();
        if (lNode)
        {
            FbxAMatrix lGlobalPosition = GetGlobalPosition(lNode);
            FbxAMatrix lGeometryOffset = GetGeometry(lNode);
            FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
            DrawLight(lNode, lGlobalOffPosition);
        }
    }
}


// Get the matrix of the given pose
FbxAMatrix 
GetPoseMatrix(
    FbxPose* pPose, 
    int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}


// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix 
GetGeometry(
    FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}


void 
DrawNodeRecursive(
    FbxNode* pNode, 
    FbxAMatrix& pParentGlobalPosition)
{
    FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, &pParentGlobalPosition);

    if (pNode->GetNodeAttribute())
    {
        // Geometry offset.
        // it is not inherited by the children.
        FbxAMatrix lGeometryOffset = GetGeometry(pNode);
        FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
        DrawNode(pNode, pParentGlobalPosition, lGlobalOffPosition);
    }

    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        DrawNodeRecursive(pNode->GetChild(lChildIndex), lGlobalPosition);
    }
}


// Draw a cross hair where the node is located.
void 
DrawNull(FbxAMatrix& pGlobalPosition)
{
    //GlDrawCrossHair(pGlobalPosition);
}

// Draw a small box where the node is located.
void 
DrawMarker(FbxAMatrix& pGlobalPosition)
{
    //GlDrawMarker(pGlobalPosition);  
}

void 
DrawNode(
    FbxNode* pNode, 
    FbxAMatrix& pParentGlobalPosition, 
    FbxAMatrix& pGlobalPosition)
{
    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

    if (lNodeAttribute)
    {
        // All lights has been processed before the whole scene because they influence every geometry.
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMarker)
        {
            DrawMarker(pGlobalPosition);
        }
        // NURBS and patch have been converted into triangluation meshes.
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            DrawMesh(pNode, pGlobalPosition);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNull)
        {
            DrawNull(pGlobalPosition);
        }
    }
    else
    {
        // Draw a Null for nodes without attribute.
        DrawNull(pGlobalPosition);
    }
}

// Draw the vertices of a mesh.
void 
DrawMesh(
    FbxNode* pNode, 
    FbxAMatrix& pGlobalPosition)
{
    FbxMesh* lMesh = pNode->GetMesh();
    const int lVertexCount = lMesh->GetControlPointsCount();

    // No vertex to draw.
    if (lVertexCount == 0)
    {
        return;
    }

    const MeshCache* lMeshCache = static_cast<const MeshCache*>(lMesh->GetUserDataPtr());

    // If it has some defomer connection, update the vertices position
    const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
	 	(static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
    const bool lHasShape = lMesh->GetShapeCount() > 0;
    const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

    FbxVector4* lVertexArray = NULL;
    if (!lMeshCache || lHasDeformation)
    {
        lVertexArray = new FbxVector4[lVertexCount];
        memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    }

    //@todo: Push this as the model matrix [jared.watt]
    GLint model_location = RenderPipeline::GetUniformLocation("model");
    if (model_location != -1) {
        GLfloat lConvertedBuffer[16];
        ConvertFbxToLinmath((const GLdouble*)pGlobalPosition, lConvertedBuffer, 16 );
        glUniformMatrix4fv(model_location, 1, GL_FALSE, lConvertedBuffer );
    }

    
    
    if (lMeshCache)
    {
        BeginMeshCacheDraw(lMeshCache);
        
        const int lSubMeshCount = lMeshCache->SubMeshes.GetCount();
        for (int lIndex = 0; lIndex < lSubMeshCount; ++lIndex)
        {
            // const FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lIndex);
            // if (lMaterial)
            // {
            //     const MaterialCache * lMaterialCache = static_cast<const MaterialCache *>(lMaterial->GetUserDataPtr());
            //     if (lMaterialCache)
            //     {
            //         SetCurrentMaterialCache( lMaterialCache );
            //     }
            // }
            // else
            // {
            //     // Draw green for faces without material
            //     SetDefaultMaterialCache();
            // }
         
            DrawMeshCache( lMeshCache, lIndex );
        }

        EndMeshCacheDraw( lMeshCache );
    }

    //glPopMatrix();

    delete [] lVertexArray;
}



// Draw a colored sphere or cone where the node is located.
void 
DrawLight(
    FbxNode* pNode,  
    FbxAMatrix& pGlobalPosition )
{
    const FbxLight* lLight = pNode->GetLight();
    if (!lLight)
    {
        return;
    }

    // Must rotate the light's global position because 
    // FBX lights point towards the Y negative axis.
    FbxAMatrix lLightRotation;
    const FbxVector4 lYNegativeAxis(-90.0, 0.0, 0.0);
    lLightRotation.SetR(lYNegativeAxis);
    const FbxAMatrix lLightGlobalPosition = pGlobalPosition * lLightRotation;

    const LightCache * lLightCache = static_cast<const LightCache *>(lLight->GetUserDataPtr());
    if (lLightCache)
    {
        RenderPipeline::BindLight(lLightCache, lLightGlobalPosition);
    }
}