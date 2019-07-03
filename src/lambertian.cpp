#include "lambertian.h"

#include "monte_carlo.h"

#include <math.h>

Lambertian::Lambertian(Color diffuse, Color emit)
    : Material(emit), m_diffuse(diffuse), m_texture(nullptr)
{}

Lambertian::Lambertian(std::shared_ptr<Texture> texture, Color emit)
    : Material(emit), m_diffuse(0.f), m_texture(texture)
{}

Color Lambertian::f(
    const Intersection &intersection,
    const Vector3 &wo,
    float *pdf
) const
{
    *pdf = UniformHemispherePdf();

    if (m_texture) {
        return m_texture->lookup(intersection.uv) / M_PI;
    } else {
        return m_diffuse / M_PI;
    }
}
