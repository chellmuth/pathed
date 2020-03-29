#pragma once

#include "color.h"
#include "random_generator.h"
#include "vector.h"

#include <ostream>
#include <tuple>

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

    virtual bool isDelta() const { return false; }
    virtual bool isContainer() const { return false; }
    virtual bool doubleSided() const { return false; }

    Color emit() const;
    virtual Color albedo(const Intersection &intersection) const {
        return Color(1.f, 0.f, 0.f);
    }

    virtual void writeStream(std::ostream &os) const {
        os << "[Material: " << m_emit << "]";
    }

    friend std::ostream &operator<<(std::ostream &os, const Material &m) {
        m.writeStream(os);
        return os;
    }

protected:
    Vector3 buildLocalWi(const Intersection &intersection) const;

    std::tuple<Vector3, Vector3> buildLocalWs(
        const Intersection &intersection,
        const Vector3 &woWorld
    ) const;

    Color m_emit;
};
