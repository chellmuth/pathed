#include "vol_parser.h"

#include "grid_medium.h"

#include <iostream>
#include <fstream>
#include <vector>

std::shared_ptr<Medium> VolParser::parse(const std::string &filename, float albedo, float scale)
{
    std::ifstream volStream(filename, std::ifstream::binary);

    std::cout << "Parsing " << filename << std::endl;

    char headerBuffer[3];
    volStream.read(headerBuffer, 3);
    std::cout << "Header: " << headerBuffer << std::endl;

    char version;
    volStream.read(reinterpret_cast<char *>(&version), sizeof(version));
    std::cout << "Version: " << (int)version << std::endl;

    uint32_t encoding;
    volStream.read(reinterpret_cast<char *>(&encoding), sizeof(encoding));
    std::cout << "Encoding: " << encoding << std::endl;

    uint32_t cellsX;
    volStream.read(reinterpret_cast<char *>(&cellsX), sizeof(encoding));
    std::cout << "cellsX: " << cellsX << std::endl;

    uint32_t cellsY;
    volStream.read(reinterpret_cast<char *>(&cellsY), sizeof(encoding));
    std::cout << "cellsY: " << cellsY << std::endl;

    uint32_t cellsZ;
    volStream.read(reinterpret_cast<char *>(&cellsZ), sizeof(encoding));
    std::cout << "cellsZ: " << cellsZ << std::endl;

    uint32_t channels;
    volStream.read(reinterpret_cast<char *>(&channels), sizeof(encoding));
    std::cout << "channels: " << channels << std::endl;

    float bounds[6];
    volStream.read(reinterpret_cast<char *>(&bounds), sizeof(bounds));
    std::cout << "bounds: " << bounds << std::endl;

    const int count = cellsX * cellsY * cellsZ;
    std::vector<float> gridData(count);
    volStream.read(reinterpret_cast<char *>(gridData.data()), sizeof(float) * count);

    std::cout << "gridData[0]: " << gridData[0] << std::endl;
    std::cout << "gridData[326910]: " << gridData[326910] << std::endl;
    std::cout << "gridData[326911]: " << gridData[326911] << std::endl;

    GridInfo gridInfo = {
        cellsX,
        cellsY,
        cellsZ,

        bounds[0],
        bounds[1],
        bounds[2],
        bounds[3],
        bounds[4],
        bounds[5]
    };

    return std::make_shared<GridMedium>(gridInfo, gridData, albedo, scale);
}
