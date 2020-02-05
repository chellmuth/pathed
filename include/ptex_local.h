#pragma once

#include "albedo.h"
#include "color.h"
#include "texture.h"
#include "uv.h"

#include <string>

class PtexLocal : public Texture {
public:
    PtexLocal(const std::string &texturePath);

    void load();
    Color lookup(UV uv) const override;
};
