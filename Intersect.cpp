
#include "Intersect.h"


NsAxisAlignedBox
Expand(const NsAxisAlignedBox& aIn, const FbxVector4& aPoint)
{
    NsAxisAlignedBox lOut;
    lOut.Min[0] = FbxMin(aIn.Min[0], aPoint[0]);
    lOut.Min[1] = FbxMin(aIn.Min[1], aPoint[1]);
    lOut.Min[2] = FbxMin(aIn.Min[2], aPoint[2]);
    lOut.Min[3] = 1.0f;
    lOut.Max[0] = FbxMax(aIn.Max[0], aPoint[0]);
    lOut.Max[1] = FbxMax(aIn.Max[1], aPoint[1]);
    lOut.Max[2] = FbxMax(aIn.Max[2], aPoint[2]);
    lOut.Max[3] = 1.0f;
    return lOut;
}


NsAxisAlignedBox 
Expand(const NsAxisAlignedBox& aFirst, const NsAxisAlignedBox& aSecond)
{
    NsAxisAlignedBox lOut;
    lOut.Min[0] = FbxMin(aFirst.Min[0], aSecond.Min[0]);
    lOut.Min[1] = FbxMin(aFirst.Min[1], aSecond.Min[1]);
    lOut.Min[2] = FbxMin(aFirst.Min[2], aSecond.Min[2]);
    lOut.Min[3] = 1.0f;
    lOut.Max[0] = FbxMax(aFirst.Max[0], aSecond.Max[0]);
    lOut.Max[1] = FbxMax(aFirst.Max[1], aSecond.Max[1]);
    lOut.Max[2] = FbxMax(aFirst.Max[2], aSecond.Max[2]);
    lOut.Max[3] = 1.0f;
    return lOut;    
}


NsAxisAlignedBox 
Transform(const NsAxisAlignedBox& aBoundingBox, FbxMatrix& aTransform)
{
    NsAxisAlignedBox lResult;

    lResult.Min = aTransform.MultNormalize(aBoundingBox.Min);
    lResult.Max = aTransform.MultNormalize(aBoundingBox.Max);

    return lResult;
}


bool 
Intersect(NsRay ray, NsPlane plane, FbxVector4* out)
{
    float denom = ray.Direction.DotProduct(plane.Normal);
    if (denom != 0) 
    {
        float t = -(ray.Origin.DotProduct(plane.Normal) + plane.Distance) / denom;
        if (t < 0) {
            return false;
        }
        if (out) {
            *out = ray.Origin + ray.Direction * t;
        }
        return true;
    }
    else if ( plane.Normal.DotProduct(ray.Origin) + plane.Distance == 0 )
    {
        *out = ray.Origin;
        return true;
    }
    
    return false;
}

bool 
Intersect(NsRay aRay, NsAxisAlignedBox aBox, FbxVector4* /*aOut*/)
{
    float tmin = ( aBox.Min[0] - aRay.Origin[0] ) / aRay.Direction[0];
    float tmax = ( aBox.Max[0] - aRay.Origin[0] ) / aRay.Direction[0];

    if ( tmin > tmax ) {
        std::swap(tmin, tmax);
    }

    float tymin = (aBox.Min[1] - aRay.Origin[1] ) / aRay.Direction[1];
    float tymax = (aBox.Max[1] - aRay.Origin[1] ) / aRay.Direction[1];

    if (tymin > tymax) {
        std::swap(tymin, tymax);
    }     

    if (tmin > tymax || tymin > tmax ) {
        return false;
    }

    if (tymin > tmin) {
        tmin = tymin;
    }

    if (tymax < tmax) {
        tmax = tymax;
    }

    float tzmin = (aBox.Min[2] - aRay.Origin[2] ) / aRay.Direction[2];
    float tzmax = (aBox.Max[2] - aRay.Origin[2] ) / aRay.Direction[2];
    if (tzmin > tzmax) {
        std::swap(tzmin, tzmax);
    }

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false; 
    }
 
    if (tzmin > tmin) {
        tmin = tzmin; 
    }
 
    if (tzmax < tmax) {
        tmax = tzmax; 
    }
 
    return true; 
}