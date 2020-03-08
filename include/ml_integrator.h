#pragma once

#include "color.h"
#include "bounce_controller.h"
#include "intersection.h"
#include "kd_tree.h"
#include "ml_pdf.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "sample.h"
#include "scene.h"
#include "vector.h"

#include <memory>
#include <vector>

class MLIntegrator : public SampleIntegrator {
public:
    MLIntegrator(BounceController bounceController);

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int pixelIndex,
        Sample &sample
    ) const override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;

private:
    Vector3 nextBounce(const Intersection &intersection, const Scene &scene, float *pdf) const;
    void createPhotons(const Scene &scene, RandomGenerator &random);
    Vector3 sample(const Vector3 &normal, RandomGenerator &random, float *pdf);
    void renderPDF(
        std::vector<float> &radianceLookup,
        const Scene &scene,
        const Intersection &intersection
    ) const;

    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    std::shared_ptr<DataSource> m_dataSource;
    std::unique_ptr<KDTree> m_KDTree;

    BounceController m_bounceController;
    MLPDFPool m_MLPDF;
};
