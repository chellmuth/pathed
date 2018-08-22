#include "integrator.h"

#include "color.h"
#include "light.h"
#include "ray.h"
#include "vector.h"

Color Integrator::Ld(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const
{
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        return emit;
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f, 0.f, 0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    Intersection shadowIntersection = scene.testIntersect(shadowRay);
    float lightDistance = lightDirection.length();

    if (shadowIntersection.hit && shadowIntersection.t + 0.0001f < lightDistance) {
        return Color(0.f, 0.f, 0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample.point, intersection.point)
        * intersection.material->f(intersection.wi, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}
