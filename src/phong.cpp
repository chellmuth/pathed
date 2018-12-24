#include "phong.h"

#include <math.h>

#include "util.h"

Phong::Phong(Color kd, Color ks, float n, Color emit)
    : Material(emit), m_kd(kd), m_ks(ks), m_n(n)
{}

Color Phong::f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) const
{
    Color diffuse = m_kd / M_PI;

    Vector3 reflected = wi.reflect(normal);
    float cosAlpha = fmaxf(0.f, wo.dot(reflected));
    Color specular = m_ks * (m_n + 2) / M_TWO_PI * powf(cosAlpha, m_n);

    return diffuse + specular;
}

float pdf(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) const {
    float diffuseProbability = wo.dot(normal) / M_PI;
    
    Vector3 reflected = wi.reflect(normal);
    float cosAlpha = fmaxf(0.f, wo.dot(reflected));
    float specularProbability = (m_n + 1) / M_TWO_PI * powf(cosAlpha, m_n);

    return m_specularSamplingWeight * specularProbability
        + (1 - m_specularSamplingWeight) * diffuseProbability;
}

Vector3 sample(RandomGenerator &random) const {
    bool chooseDiffuse = random.next() > m_specularSamplingWeight;
    if (chooseDiffuse) {
        float theta = acosf(sqrtf(random.next()));
        float phi = 2 * M_TWO_PI * random.next();

        Vector3 v(
            sinf(theta) * cosf(phi),
            sinf(theta) * sinf(phi),
            cosf(theta)
        );

        return v;
    } else {
        float a = acosf(powf(random.next(), 1/(m_n + 1)));
        float phi = M_TWO_PI * random.next();

        Vector v(
            sinf(a) * cosf(phi),
            sinf(a) * sinf(phi),
            cosf(a)
        );
    }

    float diffuseProbability = wo.dot(normal) / M_PI;
    
    Vector3 reflected = wi.reflect(normal);
    float cosAlpha = fmaxf(0.f, wo.dot(reflected));
    float specularProbability = (m_n + 1) / M_TWO_PI * powf(cosAlpha, m_n);

    return m_specularSamplingWeight * specularProbability
        + (1 -  m_specularSamplingWeight) * diffuseProbability;
}
