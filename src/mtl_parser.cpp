#include "mtl_parser.h"

#include <iostream>

using string = std::string;

MtlParser::MtlParser(const string &mtlFilename)
    : m_mtlFile(mtlFilename)
{}

void MtlParser::parse()
{
    string line;
    while(std::getline(m_mtlFile, line)) {
        parseLine(line);
    }
}

void MtlParser::parseLine(string &line)
{
    std::cout << line << std::endl;
}
