#ifndef AXISALIGNEDBOX_H
#define AXISALIGNEDBOX_H

#include "Common.h"

struct NsAxisAlignedBox {
    FbxVector4 Min;
    FbxVector4 Max;

    NsAxisAlignedBox() :
        Min(FBXSDK_DOUBLE_MAX,FBXSDK_DOUBLE_MAX,FBXSDK_DOUBLE_MAX),
        Max(FBXSDK_DOUBLE_MIN,FBXSDK_DOUBLE_MIN,FBXSDK_DOUBLE_MIN)
    {}
};

NsAxisAlignedBox Expand(const NsAxisAlignedBox& aIn, const FbxVector4& aPoint);
NsAxisAlignedBox Expand(const NsAxisAlignedBox& aFirst, const NsAxisAlignedBox& aSecond);

// @todo: [jared.watt, 22/04/20]
//  Investigate swapping to an AffineMatrix
NsAxisAlignedBox Transform(const NsAxisAlignedBox& aIn, FbxMatrix& aTransform);

#endif // AXISALIGNEDBOX_H