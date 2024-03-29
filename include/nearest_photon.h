#pragma once

#include "color.h"
#include "depositer.h"
#include "intersection.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "scene.h"

#include "nanoflann.hpp"

#include <memory>

class NearestPhoton : public SampleIntegrator {
public:
    NearestPhoton();

    void preprocess(const Scene &scene, RandomGenerator &random) override;

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int pixelIndex,
        Sample &sample
    ) const override;

    void debug(const Intersection &intersection, const Scene &scene) const override;

private:
    std::shared_ptr<DataSource> m_dataSource;
    KDTree *m_KDTree;
};
