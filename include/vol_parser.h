#pragma once

#include "medium.h"
#include "transform.h"

#include <memory>
#include <string>

class Transform;

namespace VolParser {
    std::shared_ptr<Medium> parse(
        const std::string &filename,
        float albedo,
        float scale,
        const Transform &transform
    );
};
