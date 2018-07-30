#include "string_util.h"

#include <iostream>

using string = std::string;

std::queue<string> tokenize(const string &line)
{
    std::queue<string> tokens;
    string remaining = lTrim(line);

    while(remaining.length() > 0) {
        string::size_type endContentIndex = remaining.find_first_of(" \t");
        if (endContentIndex == std::string::npos) {
            tokens.push(remaining);
            return tokens;
        }

        tokens.push(remaining.substr(0, endContentIndex));
        remaining = lTrim(remaining.substr(endContentIndex));
    }

    return tokens;
}

string lTrim(const std::string &token)
{
    string::size_type firstContentIndex = token.find_first_not_of(" \t");
    if (firstContentIndex == 0) {
        return string(token);
    } else if (firstContentIndex == std::string::npos) {
        return "";
    }

    return token.substr(firstContentIndex);
}
