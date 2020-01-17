#pragma once

#include "medium.h"
#include "obj_parser.h"

#include <memory>
#include <string>

namespace VolParser {
    std::shared_ptr<Medium> parse(
        const std::string &filename,
        float albedo,
        float scale,
        Handedness handedness
    );
};
