#include "ray.h"

class Scene {
public:
    Scene();

    bool testIntersect(const Ray &ray);
};
