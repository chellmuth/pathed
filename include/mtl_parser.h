#pragma once

#include <fstream>
#include <string>

class MtlParser {
public:
    MtlParser(const std::string &mtlFilename);

    void parse();

private:
    std::ifstream m_mtlFile;

    void parseLine(std::string &line);
};
