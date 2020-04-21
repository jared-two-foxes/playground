
#include "Intersect.h"

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