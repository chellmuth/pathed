#pragma once

#include "surface.h"
#include "transform.h"

#include <memory>
#include <vector>

namespace Quad {
    void parse(
        const Transform &transform,
        std::shared_ptr<Material> material,
        std::vector<std::shared_ptr<Surface>> &surfaces
    );
};
