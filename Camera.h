#ifndef CAMERA_H__
#define CAMERA_H__

#include "Common.h"
#include "Ray.h"

void Camera_GetViewMatrix(FbxMatrix* pMatrix);
void Camera_GetProjectionMatrix(FbxMatrix* pMatrix);

void Camera_MoveBy(double dx, double dz);
void Camera_LookAtRelative(double dx, double dy);

void Camera_SetViewportAndCamera(
    int pWindowWidth, 
    int pWindowHeight);


FbxVector4 Camera_ScreenToWorld(FbxVector4 screenCoords);

NsRay      Camera_ScreenToRay(FbxVector2 screenCoords);

#endif // CAMERA_H__