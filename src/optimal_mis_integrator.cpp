#include "optimal_mis_integrator.h"

#include "mis.h"
#include "ray.h"
#include "vector.h"
#include "world_frame.h"

Color OptimalMISIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    BSDFSample bsdfSample = intersection.material->sample(intersection, random);
    return direct(intersection, bsdfSample, scene, random, sample);
}

Color OptimalMISIntegrator::direct(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color result(0.f);

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

    const float pdfs[2][2] = {
        { lightRecord.solidAnglePDF, bsdfSample.material->pdf(intersection, lightRecord.wi) },
        { lightPDFForBSDFSample, bsdfRecord.solidAnglePDF }
    };

    return result;
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
