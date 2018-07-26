#include "string_util.h"

using string = std::string;

string lTrim(const std::string &token)
{
    string::size_type firstContentIndex = token.find_first_not_of(" \t");
    if (firstContentIndex == std::string::npos) {
        return string(token);
    }

    return token.substr(firstContentIndex);
}
