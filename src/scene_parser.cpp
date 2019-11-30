#include "scene_parser.h"

#include "area_light.h"
#include "camera.h"
#include "checkerboard.h"
#include "environment_light.h"
#include "globals.h"
#include "job.h"
#include "lambertian.h"
#include "light.h"
#include "matrix.h"
#include "mirror.h"
#include "obj_parser.h"
#include "phong.h"
#include "point.h"
#include "quad.h"
#include "scene.h"
#include "sphere.h"
#include "surface.h"
#include "transform.h"
#include "uv.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

typedef std::vector<std::vector<std::shared_ptr<Surface>>> NestedSurfaceVector;

static float parseFloat(json floatJson);
static Point3 parsePoint(json pointJson, bool flipHandedness = false);
static Vector3 parseVector(json vectorJson);
static Color parseColor(json colorJson, bool required = false);
static UV parseUV(json UVJson);
static Transform parseTransform(json transformJson);
static std::shared_ptr<Material> parseMaterial(json bsdfJson);

static void parseObjects(
    json objectsJson,
    std::vector<std::vector<std::shared_ptr<Surface> > > &surfaces,
    std::vector<std::shared_ptr<BVH> > &bvhs
);
static void parseObj(json objectJson, std::vector<std::shared_ptr<Surface>> &surfaces);
static void parseSphere(json sphereJson, std::vector<std::shared_ptr<Surface>> &surfaces);
static void parseQuad(json quadJson, std::vector<std::shared_ptr<Surface>> &surfaces);
static void parseEnvironmentLight(
    json environmentLightJson,
    std::shared_ptr<EnvironmentLight> &environmentLight
);

Scene parseScene(std::ifstream &sceneFile)
{
    json sceneJson = json::parse(sceneFile);

    auto sensor = sceneJson["sensor"];

    float fov = parseFloat(sensor["fov"]);

    Resolution resolution = { g_job->width(), g_job->height() };
    auto camera = std::make_shared<Camera>(
        parsePoint(sensor["lookAt"]["origin"], true),
        parsePoint(sensor["lookAt"]["target"], true),
        parseVector(sensor["lookAt"]["up"]),
        fov / 180.f * M_PI,
        resolution
    );

    auto objects = sceneJson["models"];
    std::vector<std::vector<std::shared_ptr<Surface> > > surfaces;
    std::vector<std::shared_ptr<BVH> > bvhs;
    parseObjects(objects, surfaces, bvhs);

    std::vector<std::shared_ptr<Light>> lights;
    for (auto &surfaceList : surfaces) {
        for (auto &surfacePtr : surfaceList) {
            if (surfacePtr->getMaterial()->emit().isBlack()) {
                continue;
            }

            auto light = std::make_shared<AreaLight>(surfacePtr);
            lights.push_back(light);
        }
    }

    std::shared_ptr<EnvironmentLight> environmentLight;
    std::shared_ptr<EnvironmentLight> environmentLight2;
    auto environmentLightJson = sceneJson["environmentLight"];
    parseEnvironmentLight(environmentLightJson, environmentLight);
    if (environmentLight) {
        lights.push_back(environmentLight);
    }

    std::vector<std::shared_ptr<Primitive>> primitives(bvhs.begin(), bvhs.end());
    Scene scene(
        primitives,
        surfaces,
        lights,
        environmentLight,
        camera
    );

    return scene;
}

static void parseObjects(
    json objectsJson,
    std::vector<std::vector<std::shared_ptr<Surface> > > &surfaces,
    std::vector<std::shared_ptr<BVH> > &bvhs
) {
    for (auto objectJson : objectsJson) {
        std::vector<std::shared_ptr<Surface>> localSurfaces;
        if (objectJson["type"] == "obj") {
            parseObj(objectJson, localSurfaces);
        } else if (objectJson["type"] == "sphere") {
            parseSphere(objectJson, localSurfaces);
        } else if (objectJson["type"] == "quad") {
            parseQuad(objectJson, localSurfaces);
        }

        auto bvh = std::make_shared<BVH>();

        std::vector<std::shared_ptr<Primitive> > primitives(
            localSurfaces.begin(), localSurfaces.end()
        );

        bvh->bake(primitives);
        bvhs.push_back(bvh);

        surfaces.push_back(localSurfaces);
    }
}

static void parseObj(json objJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    std::ifstream objFile(objJson["filename"].get<std::string>());

    auto transformJson = objJson["transform"];
    Transform transform;
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    ObjParser objParser(objFile, transform, false, Handedness::Left);
    auto objSurfaces = objParser.parse();

    std::shared_ptr<Material> jsonMaterial;
    auto bsdf = objJson["bsdf"];
    if (bsdf.is_object()) {
        jsonMaterial = parseMaterial(bsdf);
    }

    for (auto surfacePtr : objSurfaces) {
        auto shape = surfacePtr->getShape();
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

static void parseQuad(json quadJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    std::shared_ptr<Material> material;
    auto bsdf = quadJson["bsdf"];
    if (bsdf.is_object()) {
        material = parseMaterial(bsdf);
    }

    Transform transform;
    auto transformJson = quadJson["transform"];
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    Quad::parse(transform, material, surfaces);
}

static void parseEnvironmentLight(
    json environmentLightJson,
    std::shared_ptr<EnvironmentLight> &environmentLight
) {
    if (environmentLightJson.is_object()) {
        environmentLight.reset(new EnvironmentLight(
            environmentLightJson["filename"].get<std::string>()
        ));
    }
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
    } else if (bsdfJson["type"] == "mirror") {
        return std::make_shared<Mirror>();
    } else if (bsdfJson["type"] == "lambertian") {
        Color diffuse = parseColor(bsdfJson["diffuseReflectance"]);
        Color emit = parseColor(bsdfJson["emit"], false);

        if (bsdfJson["texture"].is_string()) {
            std::string texturePath = bsdfJson["texture"].get<std::string>();
            auto texture = std::make_shared<Texture>(texturePath);
            texture->load();

            return std::make_shared<Lambertian>(texture, emit);
        } else if (
            bsdfJson["albedo"].is_object()
            && bsdfJson["albedo"]["type"] == "checkerboard"
        ) {
            auto albedoJson = bsdfJson["albedo"];
            Color onColor = parseColor(albedoJson["onColor"]);
            Color offColor = parseColor(albedoJson["offColor"]);
            UV resolution = parseUV(albedoJson["resolution"]);

            auto checkerboard = std::make_shared<Checkerboard>(
                onColor, offColor, resolution
            );
            return std::make_shared<Lambertian>(checkerboard, emit);
        } else {
            return std::make_shared<Lambertian>(diffuse, emit);
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

    auto rotate = transformJson["rotate"];
    if (rotate.is_array()) {
        matrix::rotateX(
            matrix,
            parseFloat(rotate[0]) * M_PI / 180.f
        );

        matrix::rotateY(
            matrix,
            parseFloat(rotate[1]) * M_PI / 180.f
        );

        matrix::rotateZ(
            matrix,
            parseFloat(rotate[2]) * M_PI / 180.f
        );
    }

    auto translate = transformJson["translate"];
    if (translate.is_array()) {
        matrix::translate(
            matrix,
            -parseFloat(translate[0]),
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

static Point3 parsePoint(json pointJson, bool flipHandedness)
{
    const float x = stof(pointJson[0].get<std::string>());
    const float y = stof(pointJson[1].get<std::string>());
    const float z = stof(pointJson[2].get<std::string>());

    if (flipHandedness) {
        return Point3(-x, y, z);
    } else {
        return Point3(x, y, z);
    }
}

static Vector3 parseVector(json vectorJson)
{
    return Vector3(
        stof(vectorJson[0].get<std::string>()),
        stof(vectorJson[1].get<std::string>()),
        stof(vectorJson[2].get<std::string>())
    );
}

static Color parseColor(json colorJson, bool required)
{
    if (colorJson.is_array()) {
        return Color(
            stof(colorJson[0].get<std::string>()),
            stof(colorJson[1].get<std::string>()),
            stof(colorJson[2].get<std::string>())
        );
    } else if (required) {
        throw "Color required!";
    } else {
        return Color(0.f);
    }
}

static UV parseUV(json UVJson)
{
    return UV {
        stof(UVJson["u"].get<std::string>()),
        stof(UVJson["v"].get<std::string>())
    };
}
