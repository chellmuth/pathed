#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Disney : public Material {
public:
    Disney(Color diffuse);
    Disney(std::shared_ptr<Albedo> albedo);

    Color f(
        const Intersection &intersection,
        const Vector3 &wiWorld,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

    Color albedo(const Intersection &intersection) const override;

    bool doubleSided() const override { return false; }

    void writeStream(std::ostream &os) const override {
        os << "[Disney: diffuse=" << m_diffuse << "]";
    }

private:
    Color m_diffuse;
    std::shared_ptr<Albedo> m_albedo;
};
