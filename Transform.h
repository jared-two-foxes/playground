#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Common.h"


FbxVector4 Unproject(
    FbxVector4 win, 
    FbxMatrix modelView, 
    FbxMatrix projection);


#endif //TRANSFORM_H