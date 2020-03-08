#pragma once

#include "bounce_controller.h"
#include "color.h"
#include "kd_tree.h"
#include "sample_integrator.h"

#include <memory>
#include <vector>

class Depositer : public SampleIntegrator {
public:
    Depositer(BounceController bounceController);

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int pixelIndex,
        Sample &sample
    ) const override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;
    void postwave(const Scene &scene, RandomGenerator &random, int waveCount) override;

    std::vector<DataSource::Point> getPhotons() const override;
    DataSource getDataSource() const override;

    void debug(const Intersection &intersection, const Scene &scene) const override;

private:
    Color direct(
        const Intersection &intersection,
        const Color &modulation,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    void createLightPaths(const Scene &scene, RandomGenerator &random);
    Vector3 sample(
        const Point3 &point,
        const Vector3 &normal,
        const KDTree *tree,
        int phiSteps,
        int thetaSteps,
        RandomGenerator &random,
        float *pdf
    );

    void debug2(const Intersection &intersection, const Scene &scene) const;

    BounceController m_bounceController;

    std::shared_ptr<DataSource> m_dataSource;
    std::unique_ptr<KDTree> m_KDTree;

    std::shared_ptr<DataSource> m_eyeDataSource;
    std::unique_ptr<KDTree> m_eyeTree;
};
