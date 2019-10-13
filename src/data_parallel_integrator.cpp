#include "data_parallel_integrator.h"

#include "camera.h"
#include "coordinate.h"
#include "globals.h"
#include "job.h"
#include "monte_carlo.h"
#include "photon_pdf.h"
#include "util.h"

#include "omp.h"

#include <mutex>

DataParallelIntegrator::DataParallelIntegrator()
{
    m_dataSource = std::make_shared<DataSource>();
}

static Color direct(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random
) {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    if (occluded) {
        return Color(0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}

static Vector3 sample(const Vector3 &normal, RandomGenerator &random, float *pdf)
{
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(normal);
    *pdf = INV_TWO_PI;
    return hemisphereToWorld.apply(hemisphereSample);
}

void DataParallelIntegrator::createPhotons(const Scene &scene, RandomGenerator &random)
{
    const int photonSamples = g_job->photonSamples();
    const int photonBounces = g_job->photonBounces();

    for (int i = 0; i < photonSamples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        float pdf;
        Vector3 bounceDirection = sample(lightSample.normal, random, &pdf);

        if (bounceDirection.dot(lightSample.normal) < 0.f) {
            assert(false);
            break;
        }

        Ray lightRay(lightSample.point, bounceDirection);

        Color throughput = lightSample.light->getMaterial()->emit();
        for (int bounce = 0; bounce < photonBounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            throughput *= fmaxf(0.f, intersection.wi.dot(intersection.normal * -1.f));
            if (throughput.isBlack()) { break; }

            if (bounce > 0) { // don't guide towards direct lights
                m_dataSource->points.push_back({
                    intersection.point.x(),
                    intersection.point.y(),
                    intersection.point.z(),
                    lightRay.origin(),
                    throughput
                });
            }

            Vector3 hemisphereSample = UniformSampleHemisphere(random);
            Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);
            bounceDirection = hemisphereToWorld.apply(hemisphereSample);
            lightRay = Ray(intersection.point, bounceDirection);

            throughput *= intersection.material->f(intersection, bounceDirection);

            // float invPDF = 1.f / INV_TWO_PI;
            // throughput *= invPDF;
        }
    }
}

void DataParallelIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    createPhotons(scene, random);

    m_KDTree = std::make_unique<KDTree>(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree->buildIndex();

    const int portOffset = g_job->portOffset();
    if (!m_MLPDF.connectToModel(portOffset)) {
        printf("We're done here!\n");
        throw "Failure to connect";
    }
}

void DataParallelIntegrator::generateInitialIntersections(
    int rows, int cols,
    const Scene &scene,
    std::vector<Intersection> &intersections
) {
    for (int i = 0; i < rows * cols; i++) {
        int row = (int)floorf(i / cols);
        int col = i % cols;

        Ray ray = scene.getCamera()->generateRay(row, col);

        Intersection intersection = scene.testIntersect(ray);
        intersections[i] = intersection;
    }
}

void DataParallelIntegrator::generatePhotonBundles(
    int rows, int cols,
    const Scene &scene,
    const std::vector<Intersection> &intersections,
    std::vector<float> &photonBundles
) {
    const int debugSearchCount = g_job->debugSearchCount();

    for (int i = 0; i < rows * cols; i++) {
        int row = (int)floorf(i / cols);
        int col = i % cols;

        const Intersection &intersection = intersections[i];
        if (!intersection.hit) { continue; }

        auto resultIndices = std::make_shared<std::vector<size_t>>(debugSearchCount);
        std::vector<float> outDistanceSquared(debugSearchCount);

        nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
        resultSet.init(resultIndices->data(), outDistanceSquared.data());

        float queryPoint[3] = {
            intersection.point.x(),
            intersection.point.y(),
            intersection.point.z()
        };
        m_KDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

        PhotonPDF photonPDF(
            intersection.point,
            m_dataSource,
            resultIndices,
            g_job->phiSteps(),
            g_job->thetaSteps()
        );

        Transform worldToNormal = worldSpaceToNormal(
            intersection.normal,
            intersection.wi
        );

        std::vector<float> photonBundle = photonPDF.asVector(worldToNormal);

        const int photonOffset = debugSearchCount * ((row * cols) + col);
        for (int j = 0; j < debugSearchCount; j++) {
            photonBundles[photonOffset + j] = photonBundle[j];
        }
    }
}

static std::mutex pdfLock;

void DataParallelIntegrator::batchSamplePDFs(
    int rows, int cols,
    std::vector<float> &phis,
    std::vector<float> &thetas,
    std::vector<float> &pdfs,
    std::vector<float> &photonBundles
) {
    pdfLock.lock();
    m_MLPDF.batchSample(rows * cols, phis, thetas, pdfs, photonBundles);
    pdfLock.unlock();
}

void DataParallelIntegrator::batchEvalPDFs(
    int rows, int cols,
    std::vector<float> &pdfs,
    std::vector<float> &photonBundles
) {
    pdfLock.lock();
    m_MLPDF.batchEval(rows * cols, pdfs, photonBundles);
    pdfLock.unlock();
}

std::vector<float> DataParallelIntegrator::visualizePDF(
    int rows, int cols,
    int row, int col,
    const Scene &scene
) {
    const int debugSearchCount = g_job->debugSearchCount();

    const Ray ray = scene.getCamera()->generateRay(row, col);
    const Intersection intersection = scene.testIntersect(ray);

    std::vector<Intersection> intersections = { intersection };
    std::vector<float> photonBundles(debugSearchCount, -1.f);
    generatePhotonBundles(1, 1, scene, intersections, photonBundles);

    std::vector<float> pdfs(rows * cols, -1.f);

    batchEvalPDFs(
        rows, cols,
        pdfs,
        photonBundles
    );

    return pdfs;
}

void DataParallelIntegrator::calculateLighting(
    int rows, int cols,
    const Scene &scene,
    RandomGenerator &random,
    std::vector<float> &phis,
    std::vector<float> &thetas,
    std::vector<float> &pdfs,
    std::vector<Intersection> &intersections,
    std::vector<Color> &colors
) {
    for (int i = 0; i < rows * cols; i++) {
        int row = (int)floorf(i / cols);
        int col = i % cols;

        const Intersection &intersection = intersections[i];
        if (!intersection.hit) {
            continue;
        }

        Transform worldToNormal = worldSpaceToNormal(
            intersection.normal,
            intersection.wi
        );

        Transform hemisphereToWorld = normalToWorldSpace(
            intersection.normal,
            intersection.wi
        );

        Vector3 hemisphereSample = sphericalToCartesian(phis[i], thetas[i]);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);

        Ray bounceRay(
            intersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { continue; }

        Color f = intersection.material->f(intersection, bounceDirection);
        float invPDF = 1.f / pdfs[i];

        Color modulation = f
            * fmaxf(0.f, bounceDirection.dot(intersection.normal))
            * invPDF;

        colors[i] = direct(bounceIntersection, scene, random) * modulation;
    }
}

void DataParallelIntegrator::sampleImage(
    std::vector<float> &radianceLookup,
    std::vector<Sample> &sampleLookup,
    Scene &scene,
    RandomGenerator &random
) {
    const int cols = g_job->width();
    const int rows = g_job->height();
    const int debugSearchCount = g_job->debugSearchCount();

    std::vector<Intersection> intersections(rows * cols, IntersectionHelper::miss);
    std::vector<float> photonBundles(rows * cols * debugSearchCount, -1.f);
    std::vector<Color> modulations(rows * cols, Color(1.f));
    std::vector<Color> results(rows * cols, Color(0.f));

    generateInitialIntersections(rows, cols, scene, intersections);
    generatePhotonBundles(rows, cols, scene, intersections, photonBundles);

    std::vector<float> phis(rows * cols, -1.f);
    std::vector<float> thetas(rows * cols, -1.f);
    std::vector<float> pdfs(rows * cols, -1.f);

    batchSamplePDFs(rows, cols, phis, thetas, pdfs, photonBundles);

    calculateLighting(
        rows, cols,
        scene,
        random,
        phis, thetas, pdfs,
        intersections,
        results
    );

    for (int i = 0; i < rows * cols; i++) {
        int row = (int)floorf(i / cols);
        int col = i % cols;

        radianceLookup[3 * (row * cols + col) + 0] += results[i].r();
        radianceLookup[3 * (row * cols + col) + 1] += results[i].g();
        radianceLookup[3 * (row * cols + col) + 2] += results[i].b();
    }
}
