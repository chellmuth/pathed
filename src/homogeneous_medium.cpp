#include "homogeneous_medium.h"

#include "scene.h"
#include "util.h"
#include "volume_helper.h"

HomogeneousMedium::HomogeneousMedium(Color sigmaT, Color sigmaS)
    : m_sigmaT(sigmaT),
      m_sigmaS(sigmaS),
      Medium()
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

    const Color Ld = VolumeHelper::directSampleLights(*this, samplePoint, scene, random);
    const Color directTransmittance = transmittance(entryPointWorld, samplePoint);

    return Ld * directTransmittance * sigmaS(samplePoint);
}
