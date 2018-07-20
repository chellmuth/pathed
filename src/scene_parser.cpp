#include "scene_parser.h"

#include <iostream>

#include "color.h"
#include "point.h"
#include "sphere.h"

static Sphere parseSphere(json sphereJson);
static Point3 parsePoint(json pointJson);
static Color parseColor(json colorJson);

Scene parseScene(json sceneJson)
{
    std::list<Sphere> objects;

    auto jsonObjects = sceneJson["objects"];
    for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
        auto jsonObject = *it;
        if (jsonObject["type"] == "sphere") {
            objects.push_back(parseSphere(jsonObject["parameters"]));
        } else {
            std::cout << "No support for type: " << jsonObject["type"].dump() << std::endl;
        }
    }

    Point3 light = parsePoint(sceneJson["light"]);

    return Scene(objects, light);
}

static Sphere parseSphere(json sphereJson)
{
    return Sphere(
        parsePoint(sphereJson["center"]),
        sphereJson["radius"],
        parseColor(sphereJson["color"])
    );
}

static Point3 parsePoint(json pointJson)
{
    return Point3(
        pointJson["x"],
        pointJson["y"],
        pointJson["z"]
    );
}

static Color parseColor(json colorJson)
{
    return Color(
        colorJson[0],
        colorJson[1],
        colorJson[2]
    );
}
