#include "scene_parser.h"

#include <iostream>

#include "color.h"
#include "point.h"
#include "sphere.h"
#include "triangle.h"

static Sphere *parseSphere(json sphereJson);
static Triangle *parseTriangle(json triangleJson);

static Point3 parsePoint(json pointJson);
static Color parseColor(json colorJson);

Scene parseScene(json sceneJson)
{
    std::list<Shape *> objects;

    auto jsonObjects = sceneJson["objects"];
    for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
        auto jsonObject = *it;
        if (jsonObject["type"] == "sphere") {
            objects.push_back(parseSphere(jsonObject["parameters"]));
        } else if (jsonObject["type"] == "triangle") {
            objects.push_back(parseTriangle(jsonObject["parameters"]));
        } else {
            std::cout << "No support for type: " << jsonObject["type"].dump() << std::endl;
        }
    }

    Point3 light = parsePoint(sceneJson["light"]);

    return Scene(objects, light);
}

static Sphere *parseSphere(json sphereJson)
{
    return new Sphere(
        parsePoint(sphereJson["center"]),
        sphereJson["radius"],
        parseColor(sphereJson["color"])
    );
}

static Triangle *parseTriangle(json triangleJson)
{
    return new Triangle(
        parsePoint(triangleJson["v0"]),
        parsePoint(triangleJson["v1"]),
        parsePoint(triangleJson["v2"])
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
