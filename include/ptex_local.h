#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"
#include "texture.h"

#include <string>

class PtexLocal : public Texture {
public:
    PtexLocal(const std::string &texturePath);

    void load();
    Color lookup(const Intersection &intersection) const override;
};
