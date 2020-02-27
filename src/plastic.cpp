#include "plastic.h"

#include "monte_carlo.h"
#include "transform.h"

Plastic::Plastic(Color diffuse, float roughness)
    : Material(Color(0.f)),
      m_lambertian(diffuse, Color(0.f)),
      m_microfacet(roughness)
{}

Color Plastic::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    float lambertianPDF;
    float microfacetPDF;
    const Color f = m_lambertian.f(intersection, wiWorld, &lambertianPDF) +
        m_microfacet.f(intersection, wiWorld, &microfacetPDF);

    *pdf = (lambertianPDF + microfacetPDF) / 2.f;

    return f;
}

BSDFSample Plastic::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const float xi = random.next();
    if (xi > 0.5f) {
        BSDFSample sample = m_lambertian.sample(intersection, random);

        float microfacetPDF;
        Color microfacetThroughput = m_microfacet.f(intersection, sample.wiWorld, &microfacetPDF);

        return BSDFSample({
            sample.wiWorld,
            (sample.pdf + microfacetPDF) / 2.f,
            sample.throughput + microfacetThroughput,
            this
        });
    } else {
        BSDFSample sample = m_microfacet.sample(intersection, random);

        float lambertianPDF;
        Color lambertianThroughput = m_lambertian.f(intersection, sample.wiWorld, &lambertianPDF);

        return BSDFSample({
            sample.wiWorld,
            (sample.pdf + lambertianPDF) / 2.f,
            sample.throughput + lambertianThroughput,
            this
        });
    }
}
