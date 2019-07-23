#include "camera.h"

#include "geometry.h"
#include "ray.h"
#include "vector.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <stdio.h>

Camera::Camera(
    Point3 origin,
    Point3 target,
    Vector3 up,
    float verticalFOV,
    Resolution resolution
)
    : m_zNear(0.01f),
      m_origin(origin),
      m_target(target),
      m_up(up),
      m_verticalFOV(verticalFOV),
      m_resolution(resolution)
{
    m_cameraToWorld = lookAt(origin, target, up);
    m_worldToCamera = lookAtInverse(origin, target, up);
}

Ray Camera::generateRay(int row, int col) const
{
    float height = 2 * tanf(m_verticalFOV / 2) * m_zNear;
    float width = height * m_resolution.x / m_resolution.y;

    Point3 origin = Point3(0.f, 0.f, 0.f);

    float jitterX = (1.f * std::rand() / RAND_MAX) - 0.5f;
    float jitterY = (1.f * std::rand() / RAND_MAX) - 0.5f;

    Vector3 direction = Vector3(
        width * (col + 0.5f + jitterX) / m_resolution.x - width / 2.f,
        height * (row + 0.5f + jitterY) / m_resolution.y - height / 2.f,
        m_zNear
    ).normalized();

    Ray transformedRay = m_cameraToWorld.apply(Ray(origin, direction));
    return transformedRay;
}

std::optional<Pixel> Camera::calculatePixel(const Point3 &point) const
{
    const Point3 cameraPoint = m_worldToCamera.apply(point);
    const Vector3 filmDirection = -cameraPoint.toVector();

    const float filmZ = 1.f;

    const Ray filmRay = Ray(
        Point3(0.f, 0.f, 0.f),
        filmDirection
    );

    const geometry::Plane filmPlane(
        Vector3(0.f, 0.f, -1.f),
        Point3(0.f, 0.f, filmZ)
    );

    const Point3 filmPoint = geometry::intersectRayPlane(filmRay, filmPlane);

    const float height = 2 * tanf(m_verticalFOV / 2) * filmZ;
    const float width = height * m_resolution.x / m_resolution.y;

    const int pixelX = std::floor(m_resolution.x * (filmPoint.x() / width + 0.5f));
    const int pixelY = std::floor(m_resolution.y * (filmPoint.y() / height + 0.5f));

    if (pixelX < 0 || pixelX >= m_resolution.x || pixelY < 0 || pixelY >= m_resolution.y) {
        return {};
    }

    const Pixel pixel {
        pixelX,
        pixelY
    };
    return pixel;
}
