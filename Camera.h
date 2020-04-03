#ifndef CAMERA_H__
#define CAMERA_H__

#include "Common.h"

namespace Camera {

    void MoveBy(double dx, double dz);

    void LookAtRelative(double dx, double dy);

    void SetViewportAndCamera(
        int pWindowWidth, 
        int pWindowHeight);

}

#endif // CAMERA_H__