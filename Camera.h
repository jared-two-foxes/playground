#ifndef CAMERA_H__
#define CAMERA_H__

#include "Common.h"

namespace Camera {

    void SetCamera(
        FbxScene* pScene, 
        const FbxArray<FbxNode*>& pCameraArray,
        int pWindowWidth, 
        int pWindowHeight);

}

#endif // CAMERA_H__