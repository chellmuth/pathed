#pragma once

#include "color.h"
#include "bounce_controller.h"
#include "intersection.h"
#include "kd_tree.h"
#include "ml_pdf.h"
#include "phi_theta_pdf.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "sample.h"
#include "scene.h"
#include "vector.h"

#include <memory>
#include <vector>

class SelfIntegrator : public SampleIntegrator {
public:
    SelfIntegrator(BounceController bounceController);

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;

private:
    Vector3 nextBounce(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        float *pdf
    ) const;

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

    PhiThetaPDF m_gtPDF;
    BounceController m_bounceController;
};
