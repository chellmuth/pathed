#pragma once

#include "color.h"
#include "material.h"

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <queue>

struct MtlMaterial {
    MtlMaterial() : diffuse(0.f, 0.f, 0.f), emit(0.f, 0.f, 0.f) {};

    Color diffuse;
    Color emit;
};

class MtlParser {
public:
    MtlParser(const std::string &mtlFilename);

    void parse();
    std::map<std::string, std::shared_ptr<Material>> materialLookup() const {
        return m_materialLookup;
    }


private:
    std::ifstream m_mtlFile;
    std::string m_currentMaterialName;
    std::map<std::string, MtlMaterial> m_mtlLookup;
    std::map<std::string, std::shared_ptr<Material> > m_materialLookup;

    void bakeLookup();

    void parseLine(std::string &line);
    void processNewMaterial(std::queue<std::string> &arguments);
    void processDiffuse(std::queue<std::string> &arguments);
    void processEmit(std::queue<std::string> &arguments);
};
