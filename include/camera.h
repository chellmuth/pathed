#pragma once

#include "point.h"
#include "transform.h"
#include "vector.h"

class Ray;

class Camera {
public:
    Camera(Point3 origin, Point3 target, Vector3 up, float verticalFOV);

    float getVerticalFOV() const { return m_verticalFOV; }
    Point3 getOrigin() const { return m_origin; }
    Point3 getTarget() const { return m_target; }

    Ray generateRay(int row, int col, int resolutionX, int resolutionY);

private:
    Point3 m_origin;
    Point3 m_target;
    Vector3 m_up;

    float m_zNear;
    Transform m_cameraToWorld;
    float m_verticalFOV;
};
