#include "optimal_mis_integrator.h"

#include "camera.h"
#include "globals.h"
#include "mis.h"
#include "ray.h"
#include "vector.h"
#include "world_frame.h"

OptimalMISIntegrator::OptimalMISIntegrator()
{
    const int width = g_job->width();
    const int height = g_job->height();

    m_AEstimates.reserve(width * height);
    m_bEstimates.reserve(width * height);
    m_alphas.reserve(width * height);
}

void OptimalMISIntegrator::preprocessPixel(
    int row, int col,
    int width, int height,
    const Scene &scene,
    RandomGenerator &random
) {
    const int pixelIndex = row * width + col;

    const int iterationCount = 16;
    for (int i = 0; i < iterationCount; i++) {
        Sample sample;

        const Ray ray = scene.getCamera()->generateRay(row, col);

        const Intersection intersection = scene.testIntersect(ray);
        if (!intersection.hit) { return; }

        const BSDFSample bsdfSample = intersection.material->sample(intersection, random);
        updateEstimates(pixelIndex, intersection, bsdfSample, scene, random, sample);
    }

    m_alphas[pixelIndex] = solveAlpha(m_AEstimates[pixelIndex], m_bEstimates[pixelIndex]);
}

void OptimalMISIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    std::cout << "Calculating estimate of optimal weights..." << std::endl;

    const int width = g_job->width();
    const int height = g_job->height();

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            preprocessPixel(
                row, col,
                width, height,
                scene,
                random
            );
        }
    }

    std::cout << "Estimate complete!" << std::endl;
}

Color OptimalMISIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int pixelIndex,
    Sample &sample
) const {
    BSDFSample bsdfSample = intersection.material->sample(intersection, random);
    return direct(intersection, bsdfSample, m_alphas[pixelIndex], scene, random, sample);
}

Color OptimalMISIntegrator::direct(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const EVector2f &alphas,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    if (std::isnan(alphas[0]) || std::isnan(alphas[1])) { return Color(1.f, 0.f, 0.f); }

    const auto lightRecord = directSampleLights(
        intersection,
        bsdfSample,
        scene,
        random,
        sample
    );

    const auto bsdfRecord = directSampleBSDF(
        intersection,
        bsdfSample,
        scene,
        random,
        sample
    );

    float lightPDFForBSDFSample = 0.f;
    if (!bsdfRecord.f.isBlack() && bsdfRecord.bounceIntersection) {
        lightPDFForBSDFSample = scene.lightsPDF(
            intersection.point,
            *bsdfRecord.bounceIntersection,
            Measure::SolidAngle
        );
    }

    const PDFLookup allPDFs = {{
        { lightRecord.solidAnglePDF, bsdfSample.material->pdf(intersection, lightRecord.wi) },
        { lightPDFForBSDFSample, bsdfRecord.solidAnglePDF }
    }};

    const std::array<float, 2> f = { lightRecord.f.average(), bsdfRecord.f.average() };

    const float sample1Weight = computeWeight(0, alphas, allPDFs[0], f[0]);
    const float sample2Weight = computeWeight(1, alphas, allPDFs[1], f[1]);

    return (lightRecord.f * sample1Weight / allPDFs[0][0]) +
        (bsdfRecord.f * sample2Weight / allPDFs[1][1]);
}

std::vector<float> OptimalMISIntegrator::buildS(const PDFLookup &allPDFs) const
{
    std::vector<float> S;

    for (const auto &samplePDFs : allPDFs) {
        float s = 1.f / (
            samplePDFs[0] + samplePDFs[1]
        );
        S.push_back(s);
    }

    return S;
}

std::vector<EVector2f> OptimalMISIntegrator::buildW(const PDFLookup &allPDFs, const std::vector<float> &S) const
{
    std::vector<EVector2f> W;
    for (int i = 0; i < allPDFs.size(); i++) {
        const auto &samplePDFs = allPDFs[i];
        EVector2f w(samplePDFs[0], samplePDFs[1]);
        W.push_back(w * S[i]);
    }

    return W;
}

Eigen::Matrix2f OptimalMISIntegrator::estimateA(
    const std::vector<EVector2f> &W,
    const std::vector<float> &S
) const {
    Eigen::Matrix2f A(Eigen::Matrix2f::Zero());

    for (int i = 0; i < W.size(); i++) {
        const EVector2f &w = W[i];
        A += w * w.transpose();
    }

    return A;
}

EVector2f OptimalMISIntegrator::estimateb(
    const std::vector<EVector2f> &W,
    const std::vector<float> &S,
    const std::array<float, 2> &f
) const {
    EVector2f b(EVector2f::Zero());

    for (int i = 0; i < W.size(); i++) {
        b += W[i] * S[i] * f[i];
    }

    return b;
}

EVector2f OptimalMISIntegrator::solveAlpha(
    const Eigen::Matrix2f &A,
    const EVector2f &b
) const {
    const EVector2f alpha = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    return alpha;
}

float OptimalMISIntegrator::computeWeight(
    int techniqueIndex,
    const EVector2f &alphas,
    const std::array<float, 2> pdfs,
    float f
) const {
    if (f == 0.f) {
        return 0.f;
    }

    const float term1 = alphas[techniqueIndex] * pdfs[techniqueIndex] / f;
    const float term2 = pdfs[techniqueIndex] / (pdfs[0] + pdfs[1]);
    const float term3 = 1.f - (alphas[0] * pdfs[0] + alphas[1] * pdfs[1]) / f;

    return term1 + term2 * term3;
}

void OptimalMISIntegrator::updateEstimates(
    int index,
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) {
    const auto lightRecord = directSampleLights(
        intersection,
        bsdfSample,
        scene,
        random,
        sample
    );

    const auto bsdfRecord = directSampleBSDF(
        intersection,
        bsdfSample,
        scene,
        random,
        sample
    );

    float lightPDFForBSDFSample = 0.f;
    if (!bsdfRecord.f.isBlack() && bsdfRecord.bounceIntersection) {
        lightPDFForBSDFSample = scene.lightsPDF(
            intersection.point,
            *bsdfRecord.bounceIntersection,
            Measure::SolidAngle
        );
    }

    const PDFLookup allPDFs = {{
        { lightRecord.solidAnglePDF, bsdfSample.material->pdf(intersection, lightRecord.wi) },
        { lightPDFForBSDFSample, bsdfRecord.solidAnglePDF }
    }};

    const std::vector<float> S = buildS(allPDFs);
    const std::vector<EVector2f> W = buildW(allPDFs, S);
    const Eigen::Matrix2f A = estimateA(W, S);
    const std::array<float, 2> f = { lightRecord.f.average(), bsdfRecord.f.average() };
    const EVector2f b = estimateb(W, S, f);

    m_AEstimates[index] += A;
    m_bEstimates[index] += b;
}

TechniqueRecord OptimalMISIntegrator::directSampleLights(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    const LightSample lightSample = scene.sampleDirectLights(intersection.point, random);

    const Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    const Vector3 wiWorld = lightDirection.normalized();

    const Ray shadowRay = Ray(intersection.point, wiWorld);
    const float lightDistance = lightDirection.length();
    const bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    const float pdf = lightSample.solidAnglePDF(intersection.point);
    const Vector3 lightWo = -lightDirection.normalized();

    if (bsdfSample.material->isDelta()
        || occluded
        || lightSample.normal.dot(wiWorld) >= 0.f
    ) {
        return TechniqueRecord({
            lightSample.point,
            lightSample.normal,
            std::nullopt,
            wiWorld,
            pdf,
            Color(0.f)
        });
    } else {
        const Color f = lightSample.light->emit(lightWo)
            * intersection.material->f(intersection, wiWorld)
            * WorldFrame::absCosTheta(intersection.shadingNormal, wiWorld);

        return TechniqueRecord({
            lightSample.point,
            lightSample.normal,
            std::nullopt,
            wiWorld,
            pdf,
            f
        });
    }
}

TechniqueRecord OptimalMISIntegrator::directSampleBSDF(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    const Ray bounceRay(intersection.point, bsdfSample.wiWorld);
    const Intersection bounceIntersection = scene.testIntersect(bounceRay);

    if (!bounceIntersection.hit) {
        return TechniqueRecord({
            std::nullopt,
            std::nullopt,
            std::nullopt,
            bsdfSample.wiWorld,
            bsdfSample.pdf,
            Color(0.f)
        });
    }

    if (bounceIntersection.hit && bounceIntersection.isEmitter()
        && bounceIntersection.woWorld.dot(bounceIntersection.shadingNormal) >= 0.f
    ) {
        const Color f = bounceIntersection.material->emit()
            * bsdfSample.throughput
            * WorldFrame::absCosTheta(intersection.shadingNormal, bsdfSample.wiWorld);

        return TechniqueRecord({
            bounceIntersection.point,
            bounceIntersection.shadingNormal,
            bounceIntersection,
            bsdfSample.wiWorld,
            bsdfSample.pdf,
            f
        });
    } else {
        return TechniqueRecord({
            bounceIntersection.point,
            bounceIntersection.shadingNormal,
            bounceIntersection,
            bsdfSample.wiWorld,
            bsdfSample.pdf,
            Color(0.f)
        });
    }
}
