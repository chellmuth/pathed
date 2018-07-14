#include "point.h"
#include "vector.h"

class Ray {
public:
    Ray(Point3 origin, Vector3 direction);

private:
    Point3 m_origin;
    Vector3 m_direction;
};
