#include "albedo_integrator.h"

Color AlbedoIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int pixelIndex,
    Sample &sample
) const {
    return intersection.material->albedo(intersection);
}
