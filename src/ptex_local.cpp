#include "ptex_local.h"

#include <Ptexture.h>

#include <cmath>
#include <iostream>

PtexLocal::PtexLocal(const std::string &texturePath)
    : Texture(texturePath)
{}

void PtexLocal::load()
{
}

struct : public PtexErrorHandler {
    void reportError(const char *error) override { std::cout << error << std::endl; }
} errorHandler;

Color PtexLocal::lookup(UV uv) const
{
    // Handle wrapping
    float u = uv.u - (int)floorf(uv.u);
    float v = uv.v - (int)floorf(uv.v);

    Ptex::PtexCache *cache = Ptex::PtexCache::create(1, 1ull << 32, true, nullptr, &errorHandler);
    Ptex::String error;
    Ptex::PtexTexture *texture = cache->get(m_texturePath.c_str(), error);
    Ptex::PtexFilter::Options opts(Ptex::PtexFilter::FilterType::f_bspline);
    Ptex::PtexFilter *filter = Ptex::PtexFilter::getFilter(texture, opts);

}

