#include "albedo_integrator.h"

Color AlbedoIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    return intersection.material->albedo(intersection);
}
