#pragma once

#include "image.h"
#include "integrator.h"
#include "intersection.h"
#include "kd_tree.h"
#include "random_generator.h"
#include "render_status.h"
#include "scene.h"

#include <memory>
#include <string>
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

    void createPhotons(
        std::shared_ptr<DataSource> dataSource,
        const Scene &scene,
        RandomGenerator &random
    );
    void createAndSaveDataPoint(
        Image &image,
        const Scene &scene,
        RandomGenerator &random,
        int pointID
    );
    Intersection generateIntersection(const Scene &scene, RandomGenerator &random);

    void savePhotonBundle(
        std::shared_ptr<DataSource> dataSource,
        std::unique_ptr<KDTree> &KDTree,
        const Intersection &intersection,
        int pointID,
        const std::string &suffix
    );

    Vector3 sample(const Vector3 &normal, RandomGenerator &random, float *pdf);

    std::shared_ptr<DataSource> m_dataSource1;
    std::unique_ptr<KDTree> m_KDTree1;

    std::shared_ptr<DataSource> m_dataSource2;
    std::unique_ptr<KDTree> m_KDTree2;

    std::shared_ptr<DataSource> m_dataSource3;
    std::unique_ptr<KDTree> m_KDTree3;

    std::shared_ptr<DataSource> m_dataSource4;
    std::unique_ptr<KDTree> m_KDTree4;
};
