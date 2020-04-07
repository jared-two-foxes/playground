
#include "RenderObjects.h"
#include "RenderPipeline.h"


const GLfloat BLACK_COLOR[] = {0.0f, 0.0f, 0.0f, 1.0f};
const GLfloat GREEN_COLOR[] = {0.0f, 1.0f, 0.0f, 1.0f};
const GLfloat WHITE_COLOR[] = {1.0f, 1.0f, 1.0f, 1.0f};


//@todo: Maybe this information needs to be baked into the MeshCache? [jared.watt]
const int TRIANGLE_VERTEX_COUNT = 3;    
const int POSITION_STRIDE = 4;          //< Four floats for every position.
const int NORMAL_STRIDE = 3;            //< Three floats for every normal.
const int UV_STRIDE = 2;                //< Two floats for every UV.



MeshCache::MeshCache() : 
    HasNormal(false), 
    HasUV(false), 
    AllByControlPoints(true)
{
    // Reset every VBO to zero, which means no buffer.
    for (int lVBOIndex = 0; lVBOIndex < VBO_COUNT; ++lVBOIndex)
    {
        VBONames[lVBOIndex] = 0;
    }
}


void 
BeginMeshCacheDraw(const MeshCache* pMeshCache)
{
    //@todo: Update all of this to use the VertexAttribPointer stuff. [jared.watt]
    
    //@note: So we can assume that all of the attrib arrays are closely packed as 
    //       we are using interleaved arrays for each attrib.

    // Set vertex position array.
    GLint position_location = RenderPipeline::GetAttributeLocation("position");
    if (position_location != -1)
    {
        glBindBuffer(GL_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::VERTEX_VBO]);
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(position_location, POSITION_STRIDE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    // Set normal array.
    GLint normal_location = RenderPipeline::GetAttributeLocation("normal");
    if (normal_location != -1)
    {
        assert(pMeshCache->HasNormal);
        glBindBuffer(GL_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::NORMAL_VBO]);   
        glEnableVertexAttribArray(normal_location);
        glVertexAttribPointer(normal_location, NORMAL_STRIDE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    // Set UV array.
    GLint uv_location = RenderPipeline::GetAttributeLocation("uv");
    if (uv_location != -1)
    {
        assert(pMeshCache->HasUV);
        glBindBuffer(GL_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::UV_VBO]);
        glEnableVertexAttribArray(uv_location);
        glTexCoordPointer(UV_STRIDE, GL_FLOAT, 0, 0);
    }

    // Set index array.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMeshCache->VBONames[MeshCache::INDEX_VBO]);
}

void 
DrawMeshCache(const MeshCache* pMeshCache, int pMaterialIndex)
{
    // Where to start.
    GLsizei lOffset = pMeshCache->SubMeshes[pMaterialIndex].IndexOffset * sizeof(unsigned int);
    const GLsizei lElementCount = pMeshCache->SubMeshes[pMaterialIndex].TriangleCount * TRIANGLE_VERTEX_COUNT;
    glDrawElements(GL_TRIANGLES, lElementCount, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(lOffset));
}

void 
EndMeshCacheDraw(const MeshCache* pMeshCache)
{
    // Reset VBO binding.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// Draw the vertices of a mesh.
void 
DrawMeshCache(
    const MeshCache* pMeshCache, 
    FbxAMatrix& pGlobalPosition)
{
    assert(pMeshCache);

    // Bind the model location if required.
    GLint model_location = RenderPipeline::GetUniformLocation("model");
    if (model_location != -1) {
        GLfloat lConvertedBuffer[16];
        ConvertFbxToLinmath((const GLdouble*)pGlobalPosition, lConvertedBuffer, 16 );
        glUniformMatrix4fv(model_location, 1, GL_FALSE, lConvertedBuffer );
    }
    
    BeginMeshCacheDraw(pMeshCache);
    
    const int lSubMeshCount = pMeshCache->SubMeshes.size();
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
        
        DrawMeshCache( pMeshCache, lIndex );
    }

    EndMeshCacheDraw( pMeshCache );
}


ColorChannel::ColorChannel() : 
    TextureName(0)
{
    Colour[0] = 0.0f;
    Colour[1] = 0.0f;
    Colour[2] = 0.0f;
    Colour[3] = 1.0f;
}

MaterialCache::MaterialCache() : 
    Shinness(0)
{}


void 
SetCurrentMaterialCache( 
    const MaterialCache* pMaterialCache )
{
    glMaterialfv(GL_FRONT, GL_EMISSION, pMaterialCache->Emissive.Colour);
    glMaterialfv(GL_FRONT, GL_AMBIENT, pMaterialCache->Ambient.Colour);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pMaterialCache->Diffuse.Colour);
    glMaterialfv(GL_FRONT, GL_SPECULAR, pMaterialCache->Specular.Colour);
    glMaterialf(GL_FRONT, GL_SHININESS, pMaterialCache->Shinness);

    glBindTexture(GL_TEXTURE_2D, pMaterialCache->Diffuse.TextureName);
}

bool 
HasTexture( 
    const MaterialCache* pMaterialCache )
{
    return pMaterialCache->Diffuse.TextureName != 0;
}


LightCache::LightCache() : 
    Type(Point)
{}
