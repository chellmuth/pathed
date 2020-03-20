#include "homogeneous_absorption_medium.h"

#include "scene.h"
#include "util.h"
#include "volume_helper.h"

Color HomogeneousAbsorptionMedium::transmittance(const Point3 &pointA, const Point3 &pointB) const
{
    const Vector3 path = (pointB - pointA).toVector();
    return util::exp(-m_sigmaA * path.length());
}

IntegrationResult HomogeneousAbsorptionMedium::integrate(
    const Point3 &entryPointWorld,
    const Point3 &exitPointWorld,
    const Scene &scene,
    RandomGenerator &random
) const
{
    return IntegrationResult({
        false,
        Point3(0.f, 0.f, 0.f),
        transmittance(entryPointWorld, exitPointWorld),
        Color(0.f),
        Color(1.f)
    });
}
