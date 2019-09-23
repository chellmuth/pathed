#include "transform.h"

#include <math.h>
#include <stdio.h>

#include "point.h"
#include "ray.h"
#include "vector.h"

static const float identity[4][4] = {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f,
};

Transform::Transform()
    : Transform(identity)
{}

Transform::Transform(const float matrix[4][4])
{
    for (int row = 0; row < 4; row++ ) {
        for (int col = 0; col < 4; col++ ) {
            m_matrix[row][col] = matrix[row][col];
        }
    }
}

Transform Transform::transposed() const
{
    float transpose[4][4] = {
        { m_matrix[0][0], m_matrix[1][0], m_matrix[2][0], m_matrix[3][0] },
        { m_matrix[0][1], m_matrix[1][1], m_matrix[2][1], m_matrix[3][1] },
        { m_matrix[0][2], m_matrix[1][2], m_matrix[2][2], m_matrix[3][2] },
        { m_matrix[0][3], m_matrix[1][3], m_matrix[2][3], m_matrix[3][3] },
    };

    return Transform(transpose);
}

Point3 Transform::apply(const Point3 &point) const
{
    float x = point.x();
    float y = point.y();
    float z = point.z();

    return Point3(
        m_matrix[0][0] * x + m_matrix[0][1] * y + m_matrix[0][2] * z + m_matrix[0][3],
        m_matrix[1][0] * x + m_matrix[1][1] * y + m_matrix[1][2] * z + m_matrix[1][3],
        m_matrix[2][0] * x + m_matrix[2][1] * y + m_matrix[2][2] * z + m_matrix[2][3]
    );
}

Vector3 Transform::apply(const Vector3 &vector) const
{
    float x = vector.x();
    float y = vector.y();
    float z = vector.z();

    return Vector3(
        m_matrix[0][0] * x + m_matrix[0][1] * y + m_matrix[0][2] * z,
        m_matrix[1][0] * x + m_matrix[1][1] * y + m_matrix[1][2] * z,
        m_matrix[2][0] * x + m_matrix[2][1] * y + m_matrix[2][2] * z
    );
}


Transform Transform::apply(const Transform &transform) const
{
    float transformed[4][4];

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            transformed[i][j] = 0.f;
            for (int k = 0; k < 4; k++) {
                transformed[i][j] += m_matrix[i][k] * transform.m_matrix[k][j];
            }
        }
    }

    return Transform(transformed);
}

Ray Transform::apply(const Ray &ray) const
{
    return Ray(
        apply(ray.origin()),
        apply(ray.direction())
    );
}

void Transform::debug() const
{
    printf("|%8.4f %8.4f %8.4f %8.4f|\n", m_matrix[0][0], m_matrix[0][1], m_matrix[0][2], m_matrix[0][3]);
    printf("|%8.4f %8.4f %8.4f %8.4f|\n", m_matrix[1][0], m_matrix[1][1], m_matrix[1][2], m_matrix[1][3]);
    printf("|%8.4f %8.4f %8.4f %8.4f|\n", m_matrix[2][0], m_matrix[2][1], m_matrix[2][2], m_matrix[2][3]);
    printf("|%8.4f %8.4f %8.4f %8.4f|\n", m_matrix[3][0], m_matrix[3][1], m_matrix[3][2], m_matrix[3][3]);
}

Transform lookAt(const Point3 &source, const Point3 &target, const Vector3 &up)
{
    Vector3 direction = (target - source).toVector().normalized();
    Vector3 xAxis = up.normalized().cross(direction).normalized();
    Vector3 yAxis = direction.cross(xAxis);

    float matrix[4][4] {
        { xAxis.x(), yAxis.x(), direction.x(), source.x() },
        { xAxis.y(), yAxis.y(), direction.y(), source.y() },
        { xAxis.z(), yAxis.z(), direction.z(), source.z() },
        { 0.f, 0.f, 0.f, 1.f }
    };

    return Transform(matrix);
}

Transform lookAtInverse(const Point3 &source, const Point3 &target, const Vector3 &up)
{
    Vector3 direction = (target - source).toVector().normalized();
    Vector3 xAxis = up.normalized().cross(direction).normalized();
    Vector3 yAxis = direction.cross(xAxis);

    float matrix[4][4] {
        { xAxis.x(),     xAxis.y(),     xAxis.z(),     -xAxis.dot(source) },
        { yAxis.x(),     yAxis.y(),     yAxis.z(),     -yAxis.dot(source) },
        { direction.x(), direction.y(), direction.z(), -direction.dot(source) },
        { 0.f, 0.f, 0.f, 1.f }
    };

    return Transform(matrix);
}

Transform normalToWorldSpace(const Vector3 &normal, const Vector3 &rayDirection)
{
    Vector3 xAxis = normal.cross(rayDirection).normalized();
    Vector3 zAxis = normal.cross(xAxis).normalized();

    float matrix[4][4] {
        { xAxis.x(), normal.x(), zAxis.x(), 0.f },
        { xAxis.y(), normal.y(), zAxis.y(), 0.f },
        { xAxis.z(), normal.z(), zAxis.z(), 0.f },
        { 0.f, 0.f, 0.f, 1.f }
    };

    return Transform(matrix);
}

Transform normalToWorldSpace(const Vector3 &normal)
{
    Vector3 xAxis(0.f);
    if (fabsf(normal.x()) > fabsf(normal.y())) {
        xAxis = Vector3(-normal.z(), 0.f, normal.x()).normalized();
    } else {
        xAxis = Vector3(0.f, -normal.z(), normal.y()).normalized();
    }
    Vector3 zAxis = normal.cross(xAxis);

    float matrix[4][4] {
        { xAxis.x(), normal.x(), zAxis.x(), 0.f },
        { xAxis.y(), normal.y(), zAxis.y(), 0.f },
        { xAxis.z(), normal.z(), zAxis.z(), 0.f },
        { 0.f, 0.f, 0.f, 1.f }
    };

    return Transform(matrix);
}

Transform worldSpaceToNormal(const Vector3 &normal)
{
    Transform normalToWorld(normalToWorldSpace(normal));
    return normalToWorld.transposed();
}

Transform worldSpaceToNormal(const Vector3 &normal, const Vector3 &rayDirection)
{
    Transform normalToWorld(normalToWorldSpace(normal, rayDirection));
    return normalToWorld.transposed();
}
