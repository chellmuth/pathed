#pragma once

class Point3 {
public:
    Point3(float x, float y, float z);

    int x() const { return m_x; }
    int y() const { return m_y; }
    int z() const { return m_z; }

    float dot(const Point3& p);

    Point3 operator- (const Point3& v) const;

    void debug();

private:
    float m_x, m_y, m_z;
};
