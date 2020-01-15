#include "homogeneous_medium.h"

#include "scene.h"
#include "util.h"

HomogeneousMedium::HomogeneousMedium(Color sigmaT)
    : m_sigmaT(sigmaT), Medium()
{}

Color HomogeneousMedium::transmittance(const Point3 &pointA, const Point3 &pointB) const
{
    const Vector3 path = (pointB - pointA).toVector();
    return util::exp(-m_sigmaT * path.length());
}

TransmittanceQueryResult HomogeneousMedium::findTransmittance(
    const Point3 &entryPointWorld,
    const Point3 &exitPointWorld,
    float targetTransmittance
) const {
    assert(m_sigmaT.r() == m_sigmaT.g() && m_sigmaT.r() == m_sigmaT.b());

    const float distance = -m_sigmaT.r() / std::log(targetTransmittance);
    const float maxDistance = (exitPointWorld - entryPointWorld).toVector().length();
    if (distance > maxDistance) {
        return TransmittanceQueryResult({ false, -1.f });
    } else {
        return TransmittanceQueryResult({ true, distance });
    }
}

Color HomogeneousMedium::directSampleLights(
    const Point3 &point,
    const Scene &scene,
    RandomGenerator &random
) const {
    const LightSample lightSample = scene.sampleDirectLights(point, random);

    const Vector3 lightDirection = (lightSample.point - point).toVector();
    const Vector3 wiWorld = lightDirection.normalized();

    if (lightSample.normal.dot(wiWorld) >= 0.f) {
        return Color(0.f);
    }

    const Ray shadowRay = Ray(point, wiWorld);
    const float lightDistance = lightDirection.length();
    const bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    if (occluded) {
        return Color(0.f);
    } else {
        const float pdf = lightSample.solidAnglePDF(point);

        const Color transmittance = Color(1.f); // todo

        return lightSample.light->emit(lightDirection)
            * transmittance
            * 1.f / (4.f * M_PI)
            / pdf;
    }
}

Color HomogeneousMedium::integrate(
    const Point3 &entryPointWorld,
    const Point3 &exitPointWorld,
    const Scene &scene,
    RandomGenerator &random
) const
{
    assert(m_sigmaT.r() == m_sigmaT.g() && m_sigmaT.r() == m_sigmaT.b());
    const float sigmaT = m_sigmaT.r();

    const Vector3 travelVector = (exitPointWorld - entryPointWorld).toVector();
    const float distance = travelVector.length();

    const float xi = random.next();
    const float numerator = -std::log(1.f - xi * (1.f - std::exp(-sigmaT * distance)));
    const float sampleT = numerator / sigmaT;

    const Ray travelRay(entryPointWorld, travelVector.normalized());
    const Point3 samplePoint = travelRay.at(sampleT);

    const Color Ld = directSampleLights(samplePoint, scene, random);
    const Color directTransmittance = transmittance(entryPointWorld, samplePoint);
    return Ld * directTransmittance;
}
