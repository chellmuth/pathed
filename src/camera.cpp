#include "camera.h"

#include <cstdlib>
#include <math.h>
#include <stdio.h>

#include "ray.h"
#include "vector.h"

Camera::Camera(Point3 origin, Point3 target, Vector3 up, float verticalFOV)
    : m_zNear(0.01f),
      m_origin(origin),
      m_target(target),
      m_up(up),
      m_verticalFOV(verticalFOV)
{
    m_cameraToWorld = lookAt(origin, target, up);
}

Ray Camera::generateRay(int row, int col, int resolutionX, int resolutionY)
{
    float height = 2 * tanf(m_verticalFOV / 2) * m_zNear;
    float width = height * resolutionX / resolutionY;

    Point3 origin = Point3(0.f, 0.f, 0.f);

    float jitterX = (1.f * std::rand() / RAND_MAX) - 0.5f;
    float jitterY = (1.f * std::rand() / RAND_MAX) - 0.5f;

    Vector3 direction = Vector3(
        width * (col + 0.5f + jitterX) / resolutionX - width / 2.f,
        height * (row + 0.5f + jitterY) / resolutionY - height / 2.f,
        m_zNear
    ).normalized();

    Ray transformedRay = m_cameraToWorld.apply(Ray(origin, direction));
    return transformedRay;
}
