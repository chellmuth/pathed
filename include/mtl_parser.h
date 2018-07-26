#pragma once

#include <fstream>
#include <string>

using string = std::string;

class MtlParser {
public:
    MtlParser(const string &mtlFilename);

    void parse();

private:
    std::ifstream m_mtlFile;

    void parseLine(string &line);
};
