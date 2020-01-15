#pragma once

#include "color.h"
#include "medium.h"

#include <memory>
#include <string>

namespace VolParser {
    std::shared_ptr<Medium> parse(const std::string &filename, Color albedo, float scale);
};
