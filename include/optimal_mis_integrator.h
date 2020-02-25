#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "sample.h"
#include "sample_integrator.h"
#include "scene.h"
#include "vector.h"

#include <optional>

struct TechniqueRecord {
    std::optional<Point3> lightPoint;
    Vector3 wi;
    float solidAnglePDF;
    Color f;
};

class OptimalMISIntegrator : public SampleIntegrator {
public:
    OptimalMISIntegrator() {}

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
    Color direct(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    TechniqueRecord directSampleLights(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    TechniqueRecord directSampleBSDF(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;
};
