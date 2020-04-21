
#include "Transform.h"


FbxVector4 
Unproject(
    FbxVector4 win, 
    FbxMatrix modelView, 
    FbxMatrix projection)
{
    //@todo: dont use hard coded values for screen dimensions.
    const int kScreenWidth = 640, kScreenHeight = 480; 

    FbxMatrix mvp = ( projection * modelView ).Inverse();
    
    // Normalise devices coordinates
    float x = (2.0f * win[0]) / kScreenWidth - 1.0f;
    float y = 1.0f - (2.0f*win[1]) / kScreenHeight;
    
    FbxVector4 tmp(x, y, win[2], 1.0f);
    FbxVector4 obj = mvp.MultNormalize(tmp);

    obj /= obj[3];

    return obj;
}
