#include "ptex_local.h"

#include "surface.h"

#include <Ptexture.h>

#include <cmath>
#include <iostream>

static Ptex::PtexCache *cache;

struct : public PtexErrorHandler {
    void reportError(const char *error) override { std::cout << error << std::endl; }
} errorHandler;

PtexLocal::PtexLocal(const std::string &texturePath)
    : Texture(texturePath)
{
    if (!cache) {
        cache = Ptex::PtexCache::create(100, 1ull << 32, true, nullptr, &errorHandler);
    }
}

void PtexLocal::load()
{
}

Color PtexLocal::lookup(const Intersection &intersection) const
{
    const UV &uv = intersection.uv;

    // Handle wrapping
    float u = uv.u - (int)floorf(uv.u);
    float v = uv.v - (int)floorf(uv.v);

    Ptex::String error;
    Ptex::PtexTexture *texture = cache->get(m_texturePath.c_str(), error);
    Ptex::PtexFilter::Options opts(Ptex::PtexFilter::FilterType::f_bspline);
    Ptex::PtexFilter *filter = Ptex::PtexFilter::getFilter(texture, opts);

    float result[3];
    const int faceIndex = (int)std::floor(intersection.surface->getFaceIndex() / 2);
    filter->eval(result, 0, texture->numChannels(), faceIndex, uv.u, uv.v, 0.f, 0.f, 0.f, 0.f);

    filter->release();
    texture->release();

    return Color(
        std::pow(result[0], 2.2f),
        std::pow(result[1], 2.2f),
        std::pow(result[2], 2.2f)
    );
}
