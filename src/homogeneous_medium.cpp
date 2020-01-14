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

Color HomogeneousMedium::integrate(
    const Point3 &entryPointWorld,
    const Point3 &exitPointWorld,
    Scene &scene
) const
{
    return Color(0.f);
}
