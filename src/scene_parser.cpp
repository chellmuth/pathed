#include "scene_parser.h"

#include "camera.h"
#include "light.h"
#include "obj_parser.h"
#include "point.h"
#include "scene.h"
#include "sphere.h"
#include "surface.h"
#include "transform.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

static float parseFloat(json floatJson);
static Point3 parsePoint(json pointJson);
static Vector3 parseVector(json vectorJson);

static void parseObjects(json objectsJson, std::vector<std::shared_ptr<Surface>> &surfaces);
static void parseObj(json objectJson, std::vector<std::shared_ptr<Surface>> &surfaces);
static void parseSphere(json sphereJson, std::vector<std::shared_ptr<Surface>> &surfaces);

Scene parseScene(std::ifstream &sceneFile)
{
    json sceneJson = json::parse(sceneFile);

    std::vector<std::shared_ptr<Surface>> surfaces;
    auto objects = sceneJson["models"];
    parseObjects(objects, surfaces);

    auto sensor = sceneJson["sensor"];
    Transform cameraToWorld = lookAt(
        parsePoint(sensor["lookAt"]["origin"]),
        parsePoint(sensor["lookAt"]["target"]),
        parseVector(sensor["lookAt"]["up"])
    );

    float fov = parseFloat(sensor["fov"]);
    auto camera = std::make_shared<Camera>(cameraToWorld,  fov/ 180.f * M_PI);

    std::vector<std::shared_ptr<Light>> lights;
    for (auto surfacePtr : surfaces) {
        if (surfacePtr->getMaterial()->emit().isBlack()) {
            continue;
        }
        auto light = std::make_shared<Light>(surfacePtr);
        lights.push_back(light);
    }

    Scene scene(
        surfaces,
        lights,
        camera
    );

    return scene;
}

static void parseObjects(json objectsJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    for (auto objectJson : objectsJson) {
        if (objectJson["type"] == "obj") {
            parseObj(objectJson, surfaces);
        } else if (objectJson["type"] == "sphere") {
            parseSphere(objectJson, surfaces);
        }

    }
}

static void parseObj(json objJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    std::ifstream objFile(objJson["filename"].get<std::string>());

    ObjParser objParser(objFile, Handedness::Left);
    Scene objScene = objParser.parseScene();

    auto bsdfJson = objJson["bsdf"];
    Color diffuse(
        stof(bsdfJson["diffuseReflectance"][0].get<std::string>()),
        stof(bsdfJson["diffuseReflectance"][1].get<std::string>()),
        stof(bsdfJson["diffuseReflectance"][2].get<std::string>())
    );
    float specular = stof(bsdfJson["specularReflectance"].get<std::string>());
    auto material = std::make_shared<Material>(diffuse, specular, Color(0.f, 0.f, 0.f));

    for (auto surfacePtr : objScene.getSurfaces()) {
        auto surface = std::make_shared<Surface>(surfacePtr->getShape(), material);
        surfaces.push_back(surface);
    }
}

static void parseSphere(json sphereJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    auto bsdfJson = sphereJson["bsdf"];
    Color diffuse(
        stof(bsdfJson["diffuseReflectance"][0].get<std::string>()),
        stof(bsdfJson["diffuseReflectance"][1].get<std::string>()),
        stof(bsdfJson["diffuseReflectance"][2].get<std::string>())
    );
    float specular = stof(bsdfJson["specularReflectance"].get<std::string>());
    auto material = std::make_shared<Material>(diffuse, specular, Color(1.f, 1.f, 1.f));

    auto sphere = std::make_shared<Sphere>(
        parsePoint(sphereJson["center"]),
        parseFloat(sphereJson["radius"]),
        Color(0.f, 1.f, 0.f)
    );
    auto surface = std::make_shared<Surface>(sphere, material);

    surfaces.push_back(surface);
}

static float parseFloat(json floatJson)
{
    return stof(floatJson.get<std::string>());
}

static Point3 parsePoint(json pointJson)
{
    return Point3(
        stof(pointJson[0].get<std::string>()),
        stof(pointJson[1].get<std::string>()),
        stof(pointJson[2].get<std::string>())
    );
}

static Vector3 parseVector(json vectorJson)
{
    return Vector3(
        stof(vectorJson[0].get<std::string>()),
        stof(vectorJson[1].get<std::string>()),
        stof(vectorJson[2].get<std::string>())
    );
}

// #include <iostream>
// #include <vector>

// #include "color.h"
// #include "point.h"
// #include "sphere.h"
// #include "triangle.h"

// static Sphere *parseSphere(json sphereJson);
// static Triangle *parseTriangle(json triangleJson);

// static Point3 parsePoint(json pointJson);
// static Color parseColor(json colorJson);

// Scene parseScene(json sceneJson)
// {
//     std::vector<Shape *> objects;

//     auto jsonObjects = sceneJson["objects"];
//     for (json::iterator it = jsonObjects.begin(); it != jsonObjects.end(); ++it) {
//         auto jsonObject = *it;
//         if (jsonObject["type"] == "sphere") {
//             objects.push_back(parseSphere(jsonObject["parameters"]));
//         } else if (jsonObject["type"] == "triangle") {
//             objects.push_back(parseTriangle(jsonObject["parameters"]));
//         } else {
//             std::cout << "No support for type: " << jsonObject["type"].dump() << std::endl;
//         }
//     }

//     Point3 light = parsePoint(sceneJson["light"]);

//     return Scene(objects, light);
// }

// static Sphere *parseSphere(json sphereJson)
// {
//     return new Sphere(
//         parsePoint(sphereJson["center"]),
//         sphereJson["radius"],
//         parseColor(sphereJson["color"])
//     );
// }

// static Triangle *parseTriangle(json triangleJson)
// {
//     return new Triangle(
//         parsePoint(triangleJson["v0"]),
//         parsePoint(triangleJson["v1"]),
//         parsePoint(triangleJson["v2"])
//     );
// }

// static Point3 parsePoint(json pointJson)
// {
//     return Point3(
//         pointJson["x"],
//         pointJson["y"],
//         pointJson["z"]
//     );
// }

// static Color parseColor(json colorJson)
// {
//     return Color(
//         colorJson[0],
//         colorJson[1],
//         colorJson[2]
//     );
// }
