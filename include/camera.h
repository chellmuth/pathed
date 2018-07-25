#pragma once

#include "point.h"
#include "transform.h"

class Ray;

class Camera {
public:
    Camera(Transform cameraToWorld, float verticalFOV);

    Ray generateRay(int row, int col, int resolutionX, int resolutionY);

private:
    float m_zNear;
    Transform m_cameraToWorld;
    float m_verticalFOV;
};
