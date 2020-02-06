#include "mtl_parser.h"

#include "lambertian.h"
#include "material.h"
#include "string_util.h"

#include <assert.h>
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

    bakeLookup();
}

void MtlParser::bakeLookup()
{
    std::map<std::string, std::shared_ptr<Material>> materialLookup;
    for (const std::pair<std::string, MtlMaterial> &item : m_mtlLookup) {
        Color diffuse = item.second.diffuse;
        Color emit = item.second.emit;

        materialLookup[item.first] = std::make_shared<Lambertian>(
            diffuse, emit
        );
    }
    m_materialLookup = materialLookup;
}

void MtlParser::parseLine(string &line)
{
    std::queue<string> tokens = tokenize(line);
    if (tokens.empty()) { return; }

    string command = tokens.front();
    tokens.pop();

    if (command == "newmtl") {
        processNewMaterial(tokens);
    } else if (command == "Kd") {
        processDiffuse(tokens);
    } else if (command == "Ke") {
        processEmit(tokens);
    } else {
        // std::cout << "Unknown command: " << command << std::endl;
    }
}

void MtlParser::processNewMaterial(std::queue<string> &arguments)
{
    assert(arguments.size() >= 1);

    string name = arguments.front();
    m_currentMaterialName = name;

    MtlMaterial m;
    m_mtlLookup[m_currentMaterialName] = m;
}

void MtlParser::processDiffuse(std::queue<string> &arguments)
{
    assert(arguments.size() >= 3);

    float r = std::stof(arguments.front());
    arguments.pop();

    float g = std::stof(arguments.front());
    arguments.pop();

    float b = std::stof(arguments.front());
    arguments.pop();

    Color diffuse(r, g, b);
    m_mtlLookup[m_currentMaterialName].diffuse = diffuse;
}

void MtlParser::processEmit(std::queue<string> &arguments)
{
    assert(arguments.size() >= 3);

    float r = std::stof(arguments.front());
    arguments.pop();

    float g = std::stof(arguments.front());
    arguments.pop();

    float b = std::stof(arguments.front());
    arguments.pop();

    Color emit(r, g, b);
    m_mtlLookup[m_currentMaterialName].emit = emit;
}
