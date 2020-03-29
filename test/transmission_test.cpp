#include "beckmann.h"
#include "fresnel.h"
#include "intersection.h"
#include "microfacet_distribution.h"
#include "point.h"
#include "snell.h"
#include "rough_dielectric.h"
#include "vector.h"

#include "catch.hpp"

#include <cmath>
#include <memory>
#include <iostream>

#define REQUIRE_COLOR_APPROX(actual, expected) \
    REQUIRE(actual.r() == Approx(expected.r())); \
    REQUIRE(actual.g() == Approx(expected.g())); \
    REQUIRE(actual.b() == Approx(expected.b()));

#define REQUIRE_VECTOR_APPROX(actual, expected) \
    REQUIRE(actual.x() == Approx(expected.x())); \
    REQUIRE(actual.y() == Approx(expected.y())); \
    REQUIRE(actual.z() == Approx(expected.z()));

static Intersection intersectionBuilder(
    const Vector3 &woWorld,
    const Vector3 &normal,
    const Vector3 &shadingNormal,
    Material *bsdf
) {
    const Intersection intersection(
        true,
        1.f,
        Point3(0.f, 0.f, 0.f),
        woWorld,
        normal,
        shadingNormal,
        { 0.f, 0.f },
        bsdf,
        nullptr
    );
    return intersection;
}

static Intersection intersectionBuilder(const Vector3 &woWorld, Material *bsdf)
{
    const Vector3 normal(0.f, 1.f, 0.f);
    return intersectionBuilder(
        woWorld,
        normal,
        normal,
        bsdf
    );
}

TEST_CASE("smoke test", "[microfacet]") {
    RoughDielectric bsdf(
        std::make_unique<Beckmann>(0.1f),
        1.5f
    );

    const Vector3 woWorld = Vector3(1.f, 0.4, 1.f).normalized();
    const Intersection intersection = intersectionBuilder(woWorld, &bsdf);

    const Vector3 wiWorld(-0.445088f, -0.772827f, -0.452366);
    float pdf;

    const Color eval = bsdf.f(intersection, wiWorld, &pdf);
    REQUIRE_COLOR_APPROX(eval, Color(22.2333f, 22.2333f, 22.2333f));
}

TEST_CASE("snell half vector reflects both sides the same", "[microfacet]") {
    const Vector3 wo = Vector3(1.f, 0.4, 1.f).normalized();
    const Vector3 wi(-0.445088f, -0.772827f, -0.452366f);

    const Vector3 expected(0.393278f, -0.83671f, 0.381115f);
    {
        const Vector3 wh = Snell::computeHalfVector(wo, wi, 1.f, 1.f, true);

        REQUIRE_VECTOR_APPROX(wh, expected);
    }
    {
        const Vector3 wh = Snell::computeHalfVector(-wo, -wi, 1.f, 1.f, true);

        REQUIRE_VECTOR_APPROX(wh, -expected);
    }
}

TEST_CASE("both sides work the same - trivial reflection", "[microfacet]") {
    RoughDielectric bsdf1(
        std::make_unique<Beckmann>(0.2f),
        1.5f
    );

    RoughDielectric bsdf2(
        std::make_unique<Beckmann>(0.2f),
        1.f/1.5f
    );

    const Vector3 woWorld = Vector3(1.f, 0.1f, 1.f).normalized();

    const Color throughput(10.92726);
    const Vector3 wiWorld(-0.636648f, 0.297959f, -0.711267f);
    {
        const Intersection intersection = intersectionBuilder(woWorld, &bsdf1);

        float pdf;

        const Color eval = bsdf1.f(intersection, wiWorld, &pdf);
        REQUIRE_COLOR_APPROX(eval, throughput);
    }
    {
        const Vector3 woFlippedWorld = woWorld.negate();
        const Vector3 wiFlippedWorld = wiWorld.negate();

        const Intersection intersection = intersectionBuilder(woFlippedWorld, &bsdf2);

        float pdf;
        const Color eval = bsdf2.f(intersection, wiFlippedWorld, &pdf);

        REQUIRE_COLOR_APPROX(eval, throughput);
    }
}

TEST_CASE("fresnel wi and wh above", "[microfacet]") {
    const Vector3 wi = Vector3(1.f, 0.1f, 1.f).normalized();
    const Vector3 wh = Vector3(1.f, 0.3f, 1.f).normalized();

    const float etaIncident = 1.f;
    const float etaTransmitted = 1.5f;

    const float actual = Fresnel::dielectricReflectanceWalter(
        wi, wh, etaIncident, etaTransmitted
    );

    const float expected = Fresnel::dielectricReflectance(
        wi.dot(wh), etaIncident, etaTransmitted
    );

    REQUIRE(actual == Approx(expected));
}

TEST_CASE("fresnel wi and wh below", "[microfacet]") {
    const Vector3 wi = Vector3(1.f, -0.1f, 1.f).normalized();
    const Vector3 wh = Vector3(1.f, -0.3f, 1.f).normalized();

    const float etaIncident = 1.5f;
    const float etaTransmitted = 1.f;

    const float actual = Fresnel::dielectricReflectanceWalter(
        wi, wh, etaIncident, etaTransmitted
    );

    const float expected = Fresnel::dielectricReflectance(
        wi.dot(wh), etaIncident, etaTransmitted
    );

    REQUIRE(actual == Approx(expected));
}

TEST_CASE("fresnel wi below wh above", "[microfacet]") {
    const Vector3 wi = Vector3(1.f, -0.1f, 1.f).normalized();
    const Vector3 wh = Vector3(1.f, 0.3f, 1.f).normalized();

    const float etaIncident = 1.f;
    const float etaTransmitted = 1.5f;

    const float actual = Fresnel::dielectricReflectanceWalter(
        wi, wh, etaIncident, etaTransmitted
    );

    const float expected = Fresnel::dielectricReflectance(
        wi.dot(wh), etaIncident, etaTransmitted
    );

    REQUIRE(actual == Approx(expected));
}

TEST_CASE("fresnel wi above wh below", "[microfacet]") {
    const Vector3 wi = Vector3(1.f, 0.1f, 1.f).normalized();
    const Vector3 wh = Vector3(1.f, -0.3f, 1.f).normalized();

    const float etaIncident = 1.5f;
    const float etaTransmitted = 1.f;

    const float actual = Fresnel::dielectricReflectanceWalter(
        wi, wh, etaIncident, etaTransmitted
    );

    const float expected = Fresnel::dielectricReflectance(
        wi.dot(wh), etaIncident, etaTransmitted
    );

    REQUIRE(actual == Approx(expected));
}

TEST_CASE("fresnel negative wi dot wh", "[microfacet]") {
    {
        const float expected = Fresnel::dielectricReflectanceWalter(
            -0.56f, 1.f, 1.5
        );

        REQUIRE(1.f == Approx(expected));
    }
    {
        const float expected = Fresnel::dielectricReflectanceWalter(
            -0.88f, 1.f, 1.5
        );

        REQUIRE(0.0507509f == Approx(expected));
    }
}

TEST_CASE("refraction direction", "[microfacet]") {
    Vector3 wo = Snell::refract(
        Vector3(-5.96046448e-08, -0.641917706f, -0.766773522f),
        Vector3(0.0918058753f, 0.995119154f, -0.0361871757f),
        1.f,
        1.5
    );

    Vector3 expected(-0.0841451f, 0.0507944f, 1.18333f);
    REQUIRE_VECTOR_APPROX(wo, expected);
}

TEST_CASE("tungsten debugging", "[microfacet]") {
    RoughDielectric bsdf(
        std::make_unique<Beckmann>(0.1f),
        1.5f
    );

    float wi[3] = {0.126004,0.82742,0.547265};
    float wo[3] = {-0.0561083,-0.798332,0.599598};

    const Vector3 woWorld = Vector3(wi[0], wi[2], wi[1]).normalized();
    const Vector3 wiWorld(wo[0], wo[2], wo[1]);

    const Intersection intersection = intersectionBuilder(woWorld, &bsdf);

    float pdf;

    const Color eval = bsdf.f(intersection, wiWorld, &pdf);

    Color expected(0.000162923f);
    REQUIRE_COLOR_APPROX(eval, expected);
}

TEST_CASE("firefly debugging", "[microfacet]") {
    RoughDielectric bsdf(
        std::make_unique<Beckmann>(0.1f),
        1.5f
    );

    float wi[3] = { 0.135236338f, 0.862829983f, 0.487068206f };
    float wo[3] = { -0.776012957f, -0.444530785f, -0.447432995f };

    float n[3] = { 0.907894373f, -0.403632134f, -0.113176115f };
    float sn[3] = { 0.912400723f, -0.399263412f, -0.0900757313f };

    const Vector3 wiWorld(wi[0], wi[1], wi[2]);
    const Vector3 woWorld(wo[0], wo[1], wo[2]);
    const Vector3 normal(n[0], n[1], n[2]);
    const Vector3 shadingNormal(sn[0], sn[1], sn[2]);

    const Intersection intersection = intersectionBuilder(
        wiWorld,
        normal,
        shadingNormal,
        &bsdf
    );

    float pdf;

    const Color eval = bsdf.f(intersection, woWorld, &pdf);

    Color expected(4.68542f);
    REQUIRE_COLOR_APPROX(eval, expected);
}
