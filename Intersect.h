#ifndef INTERSECT_H
#define INTERSECT_H_

#include "Common.h"
#include "Plane.h"
#include "Ray.h"

// struct NsVector2 {
//     float x, y;
// };

// struct NsVector4 {
//     float x, y, z, w;
// };

bool Intersect(NsRay ray, NsPlane plane, FbxVector4* out);

#endif // INTERSECT_H