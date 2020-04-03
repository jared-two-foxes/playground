
#include "Camera.h"
#include "RenderPipeline.h"


// Set the view to the current camera settings.
void 
Camera::SetCamera(
    FbxScene* pScene, 
    const FbxArray<FbxNode*>& pCameraArray,
    int pWindowWidth, 
    int pWindowHeight)
{
    // Compute the camera position and direction.
    FbxVector4 lEye(0,75,300);
    FbxVector4 lTarget(0,75,0);
    FbxVector4 lUp(0,1,0);
    double lNearPlane = 0.01;
    double lFarPlane = 4000.0;
    double lFieldOfView = 45.0;
    double lRealScreenRatio = (double)pWindowWidth / (double)pWindowHeight;
    int  lViewPortPosX = 0, 
        lViewPortPosY = 0, 
        lViewPortSizeX = pWindowWidth, 
        lViewPortSizeY = pWindowHeight;
   
    GLint projection_location, view_location, eye_position_location;
    GLfloat lConvertedBuffer[16];

    // Calculate the Projection Matrix.
    projection_location = RenderPipeline::GetUniformLocation("projection");
    if (projection_location != -1) {
        mat4x4 lProjection;
        mat4x4_perspective(lProjection, lFieldOfView*FBXSDK_DEG_TO_RAD, lRealScreenRatio, lNearPlane, lFarPlane);
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (const GLfloat*)lProjection );
    }

    // Calculate the View Matrix.
    view_location = RenderPipeline::GetUniformLocation("view");
    if (view_location != -1) {
        FbxMatrix lView;
        lView.SetLookAtRH( lEye, lTarget, lUp );
        ConvertFbxToLinmath( (const double*)lView, lConvertedBuffer, 16 );
        glUniformMatrix4fv(view_location, 1, GL_FALSE, (const GLfloat*)lConvertedBuffer );
    }

    // Pass the eye position.
    eye_position_location = RenderPipeline::GetUniformLocation("eye_position");
    if ( eye_position_location != -1 ) {
        ConvertFbxToLinmath( (const double*)lEye, lConvertedBuffer, 4 );
        glUniform4fv(eye_position_location, 1, (const GLfloat*)lConvertedBuffer );
    }

    // Calculate the Viewport.
    glViewport( lViewPortPosX, lViewPortPosY, lViewPortSizeX, lViewPortSizeY );
}
    