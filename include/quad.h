#pragma once

#include "medium.h"
#include "surface.h"
#include "transform.h"
#include "types.h"

#include <memory>
#include <vector>

namespace Quad {
    void parse(
        const Transform &transform,
        std::shared_ptr<Material> material,
        std::shared_ptr<Medium> internalMedium,
        std::vector<std::shared_ptr<Surface>> &surfaces,
        Axis upAxis
    );
};
