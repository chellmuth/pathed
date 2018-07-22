#include "camera.h"

#include <math.h>
#include <stdio.h>

#include "ray.h"
#include "vector.h"

Camera::Camera(Point3 origin, float verticalFOV)
    : m_zNear(0.01f), m_origin(origin), m_verticalFOV(verticalFOV)
{
}

Ray Camera::generateRay(int row, int col, int resolutionX, int resolutionY)
{
    float height = 2 * tanf(m_verticalFOV / 2) * m_zNear;
    float width = height * resolutionX / resolutionY;

    Point3 origin = m_origin;
    Vector3 direction = Vector3(
        width * (col + 0.5f) / resolutionX - width / 2.f,
        height * (row + 0.5f) / resolutionY - height / 2.f,
        m_zNear
    ).normalized();

    return Ray(origin, direction);
}
