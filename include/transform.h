#pragma once

class Point3;
class Ray;
class Vector3;

class Transform {
public:
    Transform();
    Transform(const float matrix[4][4]);

    Point3 apply(const Point3 &point) const;
    Vector3 apply(const Vector3 &vector) const;
    Ray apply(const Ray &ray) const;

    void debug() const;

private:
    float m_matrix[4][4];
};

Transform lookAt(const Point3 &source, const Point3 &target, const Vector3 &up);
Transform normalToWorldSpace(const Vector3 &normal, const Vector3 &rayDirection);
