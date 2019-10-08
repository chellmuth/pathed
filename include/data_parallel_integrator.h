#pragma once

#include "integrator.h"
#include "intersection.h"
#include "kd_tree.h"
#include "ml_pdf.h"
#include "sample.h"
#include "scene.h"
#include "random_generator.h"

#include <memory>
#include <vector>

class DataParallelIntegrator : public Integrator {
public:
    DataParallelIntegrator();

protected:
    void sampleImage(
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        Scene &scene,
        RandomGenerator &random
    ) override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;

private:
    void createPhotons(const Scene &scene, RandomGenerator &random);

    void generateInitialIntersections(
        int rows, int cols,
        const Scene &scene,
        std::vector<Intersection> &intersections
    );

    void generatePhotonBundles(
        int rows, int cols,
        const Scene &scene,
        const std::vector<Intersection> &intersections,
        std::vector<float> &photonBundles
    );

    void batchSamplePDFs(
        int rows, int cols,
        std::vector<float> &phis,
        std::vector<float> &thetas,
        std::vector<float> &pdfs,
        std::vector<float> &photonBundles
    );

    void calculateLighting(
        int rows, int cols,
        const Scene &scene,
        RandomGenerator &random,
        std::vector<float> &phis,
        std::vector<float> &thetas,
        std::vector<float> &pdfs,
        std::vector<Intersection> &intersections,
        std::vector<Color> &colors
    );

    std::shared_ptr<DataSource> m_dataSource;
    std::unique_ptr<KDTree> m_KDTree;

    MLPDF m_MLPDF;
};
