#include "scene_parser.h"

#include "color.h"
#include "sphere.h"

static Sphere parseSphere(json sphereJson);
static Point3 parsePoint(json pointJson);
static Color parseColor(json colorJson);

Scene parseScene(json sceneJson)
{
    std::list<Sphere> objects;

    auto jsonObjects = sceneJson["objects"];
    for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
        objects.push_back(parseSphere((*it)["parameters"]));
    }

    return Scene(objects);
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
