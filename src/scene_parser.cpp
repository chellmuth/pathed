#include "scene_parser.h"

#include "area_light.h"
#include "camera.h"
#include "checkerboard.h"
#include "environment_light.h"
#include "glass.h"
#include "globals.h"
#include "homogeneous_medium.h"
#include "job.h"
#include "lambertian.h"
#include "light.h"
#include "matrix.h"
#include "medium.h"
#include "microfacet.h"
#include "mirror.h"
#include "obj_parser.h"
#include "oren_nayar.h"
#include "passthrough.h"
#include "perfect_transmission.h"
#include "phong.h"
#include "point.h"
#include "quad.h"
#include "scene.h"
#include "sphere.h"
#include "surface.h"
#include "texture.h"
#include "transform.h"
#include "uv.h"
#include "vector.h"
#include "vol_parser.h"

#include "json.hpp"
using json = nlohmann::json;

#include <map>

using MediaMap = std::map<std::string, std::shared_ptr<Medium> >;
using NestedSurfaceVector = std::vector<std::vector<std::shared_ptr<Surface> > >;

static bool checkFloat(json floatJson, float *value);
static float parseFloat(json floatJson);
static float parseFloat(json floatJson, float defaultValue);
static bool checkString(json stringJson, std::string *value);
static std::string parseString(json stringJson);
static Point3 parsePoint(json pointJson, bool flipHandedness = false);
static Vector3 parseVector(json vectorJson);
static Color parseColor(json colorJson, bool required = false);
static Color parseColor(json colorJson, Color defaultColor);
static UV parseUV(json UVJson);
static Transform parseTransform(json transformJson);
static std::shared_ptr<Material> parseMaterial(json bsdfJson);

static void parseMedia(
    json mediaJson,
    MediaMap &media
);
static void parseObjects(
    json objectsJson,
    std::vector<std::vector<std::shared_ptr<Surface> > > &surfaces,
    MediaMap &media
);
static void parseObj(
    json objectJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MediaMap &media
);
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

    std::map<std::string, std::shared_ptr<Medium> > media;
    auto mediaJson = sceneJson["media"];
    parseMedia(mediaJson, media);

    std::vector<std::vector<std::shared_ptr<Surface> > > surfaces;

    auto objects = sceneJson["models"];
    parseObjects(objects, surfaces, media);

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
    auto environmentLightJson = sceneJson["environmentLight"];
    parseEnvironmentLight(environmentLightJson, environmentLight);
    if (environmentLight) {
        lights.push_back(environmentLight);
    }

    Scene scene(
        surfaces,
        lights,
        environmentLight,
        camera
    );

    return scene;
}

static void parseMedia(
    json mediaJson,
    std::map<std::string, std::shared_ptr<Medium> > &media
) {
    if (!mediaJson.is_array()) { return; }

    for (auto &mediumJson : mediaJson) {
        if (mediumJson["type"] == "heterogeneous") {
            std::shared_ptr<Medium> medium = VolParser::parse(mediumJson["filename"]);
            media[mediumJson["name"]] = medium;
        } else if (mediumJson["type"] == "homogeneous") {
            Color sigmaT = parseColor(mediumJson["sigma_t"]);

            std::shared_ptr<Medium> medium = std::make_shared<HomogeneousMedium>(sigmaT);
            media[mediumJson["name"]] = medium;
        }
    }
}

static void parseObjects(
    json objectsJson,
    std::vector<std::vector<std::shared_ptr<Surface> > > &surfaces,
    MediaMap &media
) {
    for (auto objectJson : objectsJson) {
        std::vector<std::shared_ptr<Surface>> localSurfaces;
        if (objectJson["type"] == "obj") {
            parseObj(objectJson, localSurfaces, media);
        } else if (objectJson["type"] == "sphere") {
            parseSphere(objectJson, localSurfaces);
        } else if (objectJson["type"] == "quad") {
            parseQuad(objectJson, localSurfaces);
        }

        surfaces.push_back(localSurfaces);
    }
}

static void parseObj(
    json objJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MediaMap &media
) {
    std::ifstream objFile(objJson["filename"].get<std::string>());

    auto transformJson = objJson["transform"];
    Transform transform;
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    ObjParser objParser(objFile, transform, false, Handedness::Left);
    auto objSurfaces = objParser.parse();

    auto bsdfJson = objJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson));

    std::shared_ptr<Medium> mediumPtr(nullptr);
    std::string mediumKey;
    if (checkString(objJson["internal_medium"], &mediumKey)) {
        mediumPtr = media[mediumKey];
    }

    for (auto surfacePtr : objSurfaces) {
        auto shape = surfacePtr->getShape();
        if (materialPtr) {
            auto surface = std::make_shared<Surface>(shape, materialPtr, mediumPtr);
            surfaces.push_back(surface);
        } else {
            surfaces.push_back(surfacePtr);
        }
    }
}

static void parseSphere(json sphereJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    auto bsdfJson = sphereJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson));

    auto sphere = std::make_shared<Sphere>(
        parsePoint(sphereJson["center"]),
        parseFloat(sphereJson["radius"])
    );

    auto surface = std::make_shared<Surface>(sphere, materialPtr, nullptr);

    surfaces.push_back(surface);

    Transform transform;
    auto transformJson = sphereJson["transform"];
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }
    sphere->create(transform, materialPtr);
}

static void parseQuad(json quadJson, std::vector<std::shared_ptr<Surface>> &surfaces)
{
    auto bsdfJson = quadJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson));

    Transform transform;
    auto transformJson = quadJson["transform"];
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    Quad::parse(transform, materialPtr, nullptr, surfaces);
}

static void parseEnvironmentLight(
    json environmentLightJson,
    std::shared_ptr<EnvironmentLight> &environmentLight
) {
    if (environmentLightJson.is_object()) {
        environmentLight.reset(new EnvironmentLight(
            environmentLightJson["filename"].get<std::string>(),
            parseFloat(environmentLightJson["scale"], 1.f)
        ));

        std::cout << environmentLight->toString() << std::endl;
    }
}
static std::shared_ptr<Material> parseMaterial(json bsdfJson)
{
    if (!bsdfJson.is_object()) {
        return std::shared_ptr<Material>(nullptr);
    }

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
    } else if (bsdfJson["type"] == "passthrough") {
        return std::make_shared<Passthrough>();
    } else if (bsdfJson["type"] == "perfect-transmission") {
        return std::make_shared<PerfectTransmission>();
    } else if (bsdfJson["type"] == "glass") {
        float ior;
        if (checkFloat(bsdfJson["ior"], &ior)) {
            return std::make_shared<Glass>(ior);
        } else {
            return std::make_shared<Glass>();
        }
    } else if (bsdfJson["type"] == "oren-nayar") {
        Color diffuse = parseColor(bsdfJson["diffuseReflectance"], Color(1.f));
        float sigma = parseFloat(bsdfJson["sigma"]);

        return std::make_shared<OrenNayar>(diffuse, sigma);
    } else if (bsdfJson["type"] == "beckmann") {
        float alpha = parseFloat(bsdfJson["alpha"]);

        return std::make_shared<Microfacet>(alpha);
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

static bool checkFloat(json floatJson, float *value)
{
    try {
        *value = parseFloat(floatJson);
        return true;
    } catch (nlohmann::detail::type_error) {
        return false;
    }
}

static float parseFloat(json floatJson, float defaultValue)
{
    try {
        return parseFloat(floatJson);
    } catch (nlohmann::detail::type_error) {
        return defaultValue;
    }
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

static Color parseColor(json colorJson, Color defaultColor)
{
    if (colorJson.is_array()) {
        return Color(
            stof(colorJson[0].get<std::string>()),
            stof(colorJson[1].get<std::string>()),
            stof(colorJson[2].get<std::string>())
        );
    } else {
        return defaultColor;
    }
}

static UV parseUV(json UVJson)
{
    return UV {
        stof(UVJson["u"].get<std::string>()),
        stof(UVJson["v"].get<std::string>())
    };
}

static bool checkString(json stringJson, std::string *value)
{
    try {
        *value = parseString(stringJson);
        return true;
    } catch (nlohmann::detail::type_error) {
        return false;
    }
}

static std::string parseString(json stringJson)
{
    return stringJson.get<std::string>();
}
