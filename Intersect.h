#ifndef INTERSECT_H
#define INTERSECT_H_

#include "Common.h"

#include "AxisAlignedBox.h"
#include "Plane.h"
#include "Ray.h"

bool Intersect(NsRay aRay, NsPlane aPlane, FbxVector4* aOut);
bool Intersect(NsRay aRay, NsAxisAlignedBox aBox, FbxVector4* aOut);

#endif // INTERSECT_H