#pragma once

#include "point.h"

class Ray;

class Camera {
public:
    Camera(Point3 origin, float verticalFOV);

    Ray generateRay(int row, int col, int resolutionX, int resolutionY);

private:
    float m_zNear;
    float m_verticalFOV;
    Point3 m_origin;
};
