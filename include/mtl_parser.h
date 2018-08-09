#pragma once

#include <fstream>
#include <map>
#include <string>
#include <queue>

#include "color.h"

struct MtlMaterial {
    MtlMaterial() : diffuse(0.f, 0.f, 0.f), emit(0.f, 0.f, 0.f) {};

    Color diffuse;
    Color emit;
};

class MtlParser {
public:
    MtlParser(const std::string &mtlFilename);

    void parse();
    std::map<std::string, MtlMaterial> materialLookup() const { return m_materialLookup; }


private:
    std::ifstream m_mtlFile;
    std::string m_currentMaterialName;
    std::map<std::string, MtlMaterial> m_materialLookup;

    void parseLine(std::string &line);
    void processNewMaterial(std::queue<std::string> &arguments);
    void processDiffuse(std::queue<std::string> &arguments);
    void processEmit(std::queue<std::string> &arguments);
};
