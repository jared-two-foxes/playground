#ifndef PLANE_H
#define PLANE_H

#include "Common.h"

struct NsPlane {
    FbxVector4 Normal;
    float Distance;
};

#endif // PLANE_H