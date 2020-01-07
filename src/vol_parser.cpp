#include "vol_parser.h"

#include "color.h"
#include "homogeneous_medium.h"

std::shared_ptr<Medium> VolParser::parse(const std::string &filename)
{
    return std::make_shared<HomogeneousMedium>(Color(0.4f));
}
