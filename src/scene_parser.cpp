#include "scene_parser.h"

#include "camera.h"
#include "lambertian.h"
#include "light.h"
#include "matrix.h"
#include "obj_parser.h"
#include "phong.h"
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
static Color parseColor(json colorJson);
static Transform parseTransform(json transformJson);
static std::shared_ptr<Material> parseMaterial(json bsdfJson);

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

    float fov = parseFloat(sensor["fov"]);
    auto camera = std::make_shared<Camera>(
        parsePoint(sensor["lookAt"]["origin"]),
        parsePoint(sensor["lookAt"]["target"]),
        parseVector(sensor["lookAt"]["up"]),
        fov / 180.f * M_PI
    );

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

    ObjParser objParser(objFile, false, Handedness::Left);
    Scene objScene = objParser.parseScene();

    std::shared_ptr<Material> jsonMaterial;
    auto bsdf = objJson["bsdf"];
    if (bsdf.is_object()) {
        jsonMaterial = parseMaterial(bsdf);
    }

    for (auto surfacePtr : objScene.getSurfaces()) {
        auto shape = surfacePtr->getShape();
        auto transformJson = objJson["transform"];
        if (transformJson.is_object()) {
            Transform transform = parseTransform(transformJson);
            shape = shape->transform(transform);
        }
        if (jsonMaterial) {
            auto surface = std::make_shared<Surface>(shape, jsonMaterial);
            surfaces.push_back(surface);
        } else {
            surfaces.push_back(surfacePtr);
        }
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
    Color radiance = parseColor(sphereJson["radiance"]);
    auto material = std::make_shared<Lambertian>(diffuse, radiance);

    auto sphere = std::make_shared<Sphere>(
        parsePoint(sphereJson["center"]),
        parseFloat(sphereJson["radius"]),
        Color(0.f, 1.f, 0.f)
    );

    auto surface = std::make_shared<Surface>(sphere, material);

    surfaces.push_back(surface);
}

static std::shared_ptr<Material> parseMaterial(json bsdfJson)
{
    if (bsdfJson["type"] == "phong") {
        Color diffuse = parseColor(bsdfJson["diffuseReflectance"]);
        Color specular = parseColor(bsdfJson["specularReflectance"]);
        return std::make_shared<Phong>(
            diffuse,
            specular,
            1000,
            Color(0.f, 0.f, 0.f)
        );
    } else if (bsdfJson["type"] == "lambertian") {
        Color diffuse = parseColor(bsdfJson["diffuseReflectance"]);

        if (bsdfJson["texture"].is_string()) {
            std::string texturePath = bsdfJson["texture"].get<std::string>();
            std::shared_ptr<Texture> texture = std::make_shared<Texture>(texturePath);
            texture->load();

            return std::make_shared<Lambertian>(texture, Color(0.f));
        } else {
            return std::make_shared<Lambertian>(
                diffuse,
                Color(0.f, 0.f, 0.f)
            );
        }
    } else {
        throw "Unimplemented";
    }
}

static Transform parseTransform(json transformJson)
{
    float matrix[4][4];
    matrix::makeIdentity(matrix);

    auto scale = transformJson["scale"];
    if (scale.is_array()) {
        matrix::scale(
            matrix,
            parseFloat(scale[0]),
            parseFloat(scale[1]),
            parseFloat(scale[2])
        );
    }

    auto translate = transformJson["translate"];
    if (translate.is_array()) {
        matrix::translate(
            matrix,
            parseFloat(translate[0]),
            parseFloat(translate[1]),
            parseFloat(translate[2])
        );
    }

    return Transform(matrix);
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

static Color parseColor(json colorJson)
{
    return Color(
        stof(colorJson[0].get<std::string>()),
        stof(colorJson[1].get<std::string>()),
        stof(colorJson[2].get<std::string>())
    );
}
