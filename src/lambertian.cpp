#include "lambertian.h"

#include "monte_carlo.h"
#include "transform.h"

#include <iostream>
#include <math.h>

Lambertian::Lambertian(Color diffuse, Color emit)
    : Material(emit), m_diffuse(diffuse), m_albedo(nullptr)
{}

Lambertian::Lambertian(std::shared_ptr<Albedo> albedo, Color emit)
    : Material(emit), m_diffuse(0.f), m_albedo(albedo)
{}

Color Lambertian::f(
    const Intersection &intersection,
    const Vector3 &wi,
    float *pdf
) const
{
    if (intersection.wo.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    *pdf = UniformHemispherePdf(wi);

    if (m_albedo) {
        return m_albedo->lookup(intersection.uv) / M_PI;
    } else {
        return m_diffuse / M_PI;
    }
}

BSDFSample Lambertian::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Transform tangentToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.wo
    );

    Vector3 localSample = CosineSampleHemisphere(random);
    // std::cout << "local sample: " << localSample.toString() << std::endl;
    // std::cout << "world sample: " << tangentToWorld.apply(localSample).toString() << std::endl;

    Vector3 worldSample = tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wi = worldSample,
        .pdf = CosineHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample)
    };

    return sample;
}
