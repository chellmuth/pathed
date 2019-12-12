#pragma once

#include "color.h"
#include "random_generator.h"
#include "vector.h"

class Material;
class Scene;
struct Intersection;

struct BSDFSample {
    Vector3 wiWorld;
    float pdf;
    Color throughput;
    const Material *material;
};

class Material {
public:
    Material(Color emit);

    virtual Color f(
        const Intersection &intersection,
        const Vector3 &wiWorld,
        float *pdf
    ) const = 0;

    Color f(const Intersection &intersection, const Vector3 &wiWorld) const {
        float pdf;
        return f(intersection, wiWorld, &pdf);
    }

    float pdf(
        const Intersection &intersection,
        const Vector3 &wiWorld
    ) const {
        float pdf;
        f(intersection, wiWorld, &pdf);
        return pdf;
    }

    virtual BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const = 0;

    Color emit() const;

protected:
    Color m_emit;
};
