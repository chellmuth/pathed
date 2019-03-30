#pragma once

#include <iostream>
#include <fstream>

class Scene;

Scene parseScene(std::ifstream &sceneFile);
