#pragma once

#include "color.h"
#include "bounce_controller.h"
#include "intersection.h"
#include "kd_tree.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "sample.h"
#include "scene.h"
#include "vector.h"

#include <memory>

class MLIntegrator : public SampleIntegrator {
public:
    MLIntegrator(BounceController bounceController);

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;

private:
    void createPhotons(const Scene &scene, RandomGenerator &random);
    Vector3 sample(const Vector3 &normal, RandomGenerator &random, float *pdf);

    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    std::shared_ptr<DataSource> m_dataSource;
    std::unique_ptr<KDTree> m_KDTree;

    BounceController m_bounceController;
};
