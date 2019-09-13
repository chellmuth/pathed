#pragma once

#include "image.h"
#include "integrator.h"
#include "intersection.h"
#include "kd_tree.h"
#include "random_generator.h"
#include "render_status.h"
#include "scene.h"

#include <memory>
#include <vector>

class PDFIntegrator : public Integrator {
public:
    PDFIntegrator();

    void run(
        Image &image,
        Scene &scene,
        std::function<void(RenderStatus)> callback,
        bool *quit
    ) override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;

protected:
    void renderPDF(
        std::vector<float> &radianceLookup,
        const Scene &scene,
        const Intersection &intersection
    );

    void createPhotons(const Scene &scene, RandomGenerator &random);
    void savePhotonBundle(const Intersection &intersection);

    Vector3 sample(const Vector3 &normal, RandomGenerator &random, float *pdf);

    std::shared_ptr<DataSource> m_dataSource;
    std::unique_ptr<KDTree> m_KDTree;
};
