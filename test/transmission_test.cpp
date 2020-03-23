#include "beckmann2.h"
#include "fresnel.h"
#include "intersection.h"
#include "microfacet_distribution.h"
#include "point.h"
#include "snell.h"
#include "rough_transmission.h"
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

static Intersection intersectionBuilder(const Vector3 &woWorld, Material *bsdf)
{
    const Vector3 normal(0.f, 1.f, 0.f);
    const Intersection intersection(
        true,
        1.f,
        Point3(0.f, 0.f, 0.f),
        woWorld,
        normal,
        normal,
        { 0.f, 0.f },
        bsdf,
        nullptr
    );
    return intersection;
}

TEST_CASE("smoke test", "[microfacet]") {
    RoughTransmission bsdf(
        std::make_unique<Beckmann2>(0.1f),
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
    RoughTransmission bsdf1(
        std::make_unique<Beckmann2>(0.2f),
        1.5f
    );

    RoughTransmission bsdf2(
        std::make_unique<Beckmann2>(0.2f),
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
