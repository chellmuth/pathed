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

#include <Eigen/Dense>

#include <array>
#include <memory>
#include <optional>

using EVector2f = Eigen::Matrix<float, 2, 1>;

struct TechniqueRecord {
    std::optional<Point3> lightPoint;
    std::optional<Vector3> lightNormal;
    std::optional<Intersection> bounceIntersection;
    Vector3 wi;
    float solidAnglePDF;
    Color f;
};

class OptimalMISIntegrator : public SampleIntegrator {
public:
    OptimalMISIntegrator();

    void preprocess(const Scene &scene, RandomGenerator &random) override;

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
    std::vector<Eigen::Matrix2f> m_AEstimates;
    std::vector<EVector2f> m_bEstimates;

    using PDFLookup = std::array<std::array<float, 2>, 2>;

    std::vector<float> buildS(const PDFLookup &allPDFs) const;
    std::vector<EVector2f> buildW(const PDFLookup &allPDFs, const std::vector<float> &S) const;
    Eigen::Matrix2f estimateA(
        const std::vector<EVector2f> &W,
        const std::vector<float> &S
    ) const;
    EVector2f estimateb(
        const std::vector<EVector2f> &W,
        const std::vector<float> &S,
        const std::array<float, 2> &f
    ) const;
    EVector2f solveAlpha(
        const Eigen::Matrix2f &A,
        const EVector2f &b
    ) const;
    std::vector<float> computeWeights(
        const EVector2f &alpha,
        const std::array<float, 2> pdfs,
        float f
    ) const;

    void preprocessPixel(
        int row, int col,
        int width, int height,
        const Scene &scene,
        RandomGenerator &random
    );

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
