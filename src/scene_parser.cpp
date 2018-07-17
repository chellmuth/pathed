#include "scene_parser.h"

#include "sphere.h"

Scene parseScene(json sceneJson)
{
    std::list<Sphere> objects;

    auto jsonObjects = sceneJson["objects"];
    for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
        objects.push_back(Sphere((*it)["parameters"]));
    }

    return Scene(objects);
}
