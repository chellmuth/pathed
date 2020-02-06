#include "scene_parser.h"

#include "area_light.h"
#include "b_spline_parser.h"
#include "camera.h"
#include "checkerboard.h"
#include "curve_parser.h"
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
#include "ply_parser.h"
#include "ptex_local.h"
#include "quad.h"
#include "rtc_manager.h"
#include "scene.h"
#include "sphere.h"
#include "surface.h"
#include "texture.h"
#include "transform.h"
#include "types.h"
#include "uv.h"
#include "vector.h"
#include "vol_parser.h"

#include "json.hpp"
using json = nlohmann::json;

#include <map>

using MediaMap = std::map<std::string, std::shared_ptr<Medium> >;
using InstanceMap = std::map<std::string, RTCScene>;
using MaterialMap = std::map<std::string, std::shared_ptr<Material> >;

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
static Transform parseTransform(json transformJson, Transform defaultTransform);
static std::shared_ptr<Material> parseMaterial(json bsdfJson, MaterialMap &materialLookup);

static void parseMedia(
    json mediaJson,
    MediaMap &media
);
static void parseMaterials(
    json materialsJson,
    MaterialMap &materialLookup
);
static void parseInstance(
    json instanceJson,
    MaterialMap &materialLookup,
    MediaMap &media,
    InstanceMap &instanceLookup,
    RTCManager &rtcManager
);
static void parseInstanced(
    json instanceJson,
    RTCScene rtcCurrentScene,
    InstanceMap &instanceLookup,
    RTCManager &rtcManager
);
static void parseObjects(
    json objectsJson,
    RTCScene rtcCurrentScene,
    MaterialMap &materialLookup,
    MediaMap &media,
    InstanceMap &instanceLoop,
    RTCManager &rtcManager
);
static void parseObj(
    json objJson,
    RTCScene rtcCurrentScene,
    MaterialMap &materialLookup,
    MediaMap &media,
    RTCManager &rtcManager
);
static void parsePLY(
    json objectJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MaterialMap &materialLookup,
    MediaMap &media
);
static void parseCurve(
    json curveJson,
    std::vector<std::shared_ptr<Surface> > &surfaces,
    MaterialMap &materialLookup
);
static void parseBSpline(
    json splineJson,
    RTCScene rtcCurrentScene,
    MaterialMap &materialLookup,
    RTCManager &rtcManager
);
static void parseSphere(
    json sphereJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MaterialMap &materialLookup,
    MediaMap &media
);
static void parseQuad(
    json quadJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MaterialMap &materialLookup
);
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

    auto rtcManagerPtr = std::make_unique<RTCManager>(g_rtcScene);

    MaterialMap materialLookup;
    parseMaterials(sceneJson["materials"], materialLookup);

    MediaMap media;
    auto mediaJson = sceneJson["media"];
    parseMedia(mediaJson, media);

    InstanceMap instanceLookup;
    auto objects = sceneJson["models"];
    parseObjects(objects, g_rtcScene, materialLookup, media, instanceLookup, *rtcManagerPtr);

    std::vector<std::shared_ptr<Light>> lights;
    for (auto &surfaceList : rtcManagerPtr->getSurfaces()) {
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
        std::move(rtcManagerPtr),
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
            const float albedo = parseFloat(mediumJson["albedo"]);
            const float scale = parseFloat(mediumJson["scale"], 1.f);

            auto mediumPtr = VolParser::parse(
                mediumJson["filename"],
                albedo,
                scale,
                parseTransform(mediumJson["transform"], Transform()),
                Handedness::Left
            );

            media[mediumJson["name"]] = mediumPtr;
        } else if (mediumJson["type"] == "homogeneous") {
            Color sigmaT = parseColor(mediumJson["sigma_t"]);
            Color sigmaS = parseColor(mediumJson["sigma_s"], Color(0.f));

            auto mediumPtr = std::make_shared<HomogeneousMedium>(sigmaT, sigmaS);
            media[mediumJson["name"]] = mediumPtr;
        }
    }
}

static void parseInstance(
    json instanceJson,
    MaterialMap &materialLookup,
    MediaMap &media,
    InstanceMap &instanceLookup,
    RTCManager &rtcManager
) {
    RTCScene rtcInstanceScene = rtcNewScene(g_rtcDevice);
    parseObjects(
        instanceJson["models"],
        rtcInstanceScene,
        materialLookup,
        media,
        instanceLookup,
        rtcManager
    );
    instanceLookup[parseString(instanceJson["name"])] = rtcInstanceScene;
    rtcCommitScene(rtcInstanceScene);
}

static void parseObjects(
    json objectsJson,
    RTCScene rtcCurrentScene,
    MaterialMap &materialLookup,
    MediaMap &media,
    InstanceMap &instanceLookup,
    RTCManager &rtcManager
) {
    for (auto objectJson : objectsJson) {
        bool needsRegistration = true;

        std::vector<std::shared_ptr<Surface>> localSurfaces;
        if (objectJson["type"] == "obj") {
            parseObj(objectJson, rtcCurrentScene, materialLookup, media, rtcManager);
            needsRegistration = false;
        } else if (objectJson["type"] == "ply") {
            parsePLY(objectJson, localSurfaces, materialLookup, media);
        } else if (objectJson["type"] == "sphere") {
            parseSphere(objectJson, localSurfaces, materialLookup, media);
        } else if (objectJson["type"] == "quad") {
            parseQuad(objectJson, localSurfaces, materialLookup);
        } else if (objectJson["type"] == "pbrt-curve") {
            parseCurve(objectJson, localSurfaces, materialLookup);
        } else if (objectJson["type"] == "b-spline") {
            parseBSpline(objectJson, rtcCurrentScene, materialLookup, rtcManager);
            needsRegistration = false;
        } else if (objectJson["type"] == "instance") {
            parseInstance(objectJson, materialLookup, media, instanceLookup, rtcManager);
            needsRegistration = false;
        } else if (objectJson["type"] == "instanced") {
            parseInstanced(objectJson, rtcCurrentScene, instanceLookup, rtcManager);
            needsRegistration = false;
        }

        if (needsRegistration) {
            rtcManager.registerSurfaces(rtcCurrentScene, localSurfaces);
        }
    }
}

static void parseObj(
    json objJson,
    RTCScene rtcCurrentScene,
    MaterialMap &materialLookup,
    MediaMap &media,
    RTCManager &rtcManager
) {
    std::vector<std::shared_ptr<Surface>> localSurfaces;

    std::ifstream objFile(objJson["filename"].get<std::string>());

    auto transformJson = objJson["transform"];
    Transform transform;
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    materialLookup["pandanusLo_leavesBot_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/pandanusLo_leavesBot_geo.ptx"), Color(0.f));
    materialLookup["pandanusLo_leavesTop_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/pandanusLo_leavesTop_geo.ptx"), Color(0.f));
    materialLookup["root_temp0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0001_geo.ptx"), Color(0.f));
    materialLookup["root_temp0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0002_geo.ptx"), Color(0.f));
    materialLookup["root_temp0003_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0003_geo.ptx"), Color(0.f));
    materialLookup["root_temp0004_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0004_geo.ptx"), Color(0.f));
    materialLookup["root_temp0005_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0005_geo.ptx"), Color(0.f));
    materialLookup["root_temp0006_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0006_geo.ptx"), Color(0.f));
    materialLookup["root_temp0007_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0007_geo.ptx"), Color(0.f));
    materialLookup["root_temp0008_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0008_geo.ptx"), Color(0.f));
    materialLookup["root_temp0009_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0009_geo.ptx"), Color(0.f));
    materialLookup["root_temp0010_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0010_geo.ptx"), Color(0.f));
    materialLookup["root_temp0011_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0011_geo.ptx"), Color(0.f));
    materialLookup["root_temp0012_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0012_geo.ptx"), Color(0.f));
    materialLookup["root_temp0013_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0013_geo.ptx"), Color(0.f));
    materialLookup["root_temp0014_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0014_geo.ptx"), Color(0.f));
    materialLookup["root_temp0015_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0015_geo.ptx"), Color(0.f));
    materialLookup["root_temp0016_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0016_geo.ptx"), Color(0.f));
    materialLookup["root_temp0017_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0017_geo.ptx"), Color(0.f));
    materialLookup["root_temp0018_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0018_geo.ptx"), Color(0.f));
    materialLookup["root_temp0019_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0019_geo.ptx"), Color(0.f));
    materialLookup["root_temp0020_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0020_geo.ptx"), Color(0.f));
    materialLookup["root_temp0021_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0021_geo.ptx"), Color(0.f));
    materialLookup["root_temp0022_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0022_geo.ptx"), Color(0.f));
    materialLookup["root_temp0023_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0023_geo.ptx"), Color(0.f));
    materialLookup["root_temp0024_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0024_geo.ptx"), Color(0.f));
    materialLookup["root_temp0025_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0025_geo.ptx"), Color(0.f));
    materialLookup["root_temp0026_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0026_geo.ptx"), Color(0.f));
    materialLookup["root_temp0027_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0027_geo.ptx"), Color(0.f));
    materialLookup["root_temp0028_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0028_geo.ptx"), Color(0.f));
    materialLookup["root_temp0029_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0029_geo.ptx"), Color(0.f));
    materialLookup["root_temp0030_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0030_geo.ptx"), Color(0.f));
    materialLookup["root_temp0031_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0031_geo.ptx"), Color(0.f));
    materialLookup["root_temp0032_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0032_geo.ptx"), Color(0.f));
    materialLookup["root_temp0033_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0033_geo.ptx"), Color(0.f));
    materialLookup["root_temp0034_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0034_geo.ptx"), Color(0.f));
    materialLookup["root_temp0035_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0035_geo.ptx"), Color(0.f));
    materialLookup["root_temp0036_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0036_geo.ptx"), Color(0.f));
    materialLookup["root_temp0037_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0037_geo.ptx"), Color(0.f));
    materialLookup["root_temp0038_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0038_geo.ptx"), Color(0.f));
    materialLookup["root_temp0039_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0039_geo.ptx"), Color(0.f));
    materialLookup["root_temp0040_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0040_geo.ptx"), Color(0.f));
    materialLookup["root_temp0041_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0041_geo.ptx"), Color(0.f));
    materialLookup["root_temp0042_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0042_geo.ptx"), Color(0.f));
    materialLookup["root_temp0043_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0043_geo.ptx"), Color(0.f));
    materialLookup["root_temp0044_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0044_geo.ptx"), Color(0.f));
    materialLookup["root_temp0045_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0045_geo.ptx"), Color(0.f));
    materialLookup["root_temp0046_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0046_geo.ptx"), Color(0.f));
    materialLookup["root_temp0047_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0047_geo.ptx"), Color(0.f));
    materialLookup["root_temp0048_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0048_geo.ptx"), Color(0.f));
    materialLookup["root_temp0049_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0049_geo.ptx"), Color(0.f));
    materialLookup["root_temp0050_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0050_geo.ptx"), Color(0.f));
    materialLookup["root_temp0051_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0051_geo.ptx"), Color(0.f));
    materialLookup["root_temp0052_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0052_geo.ptx"), Color(0.f));
    materialLookup["root_temp0053_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0053_geo.ptx"), Color(0.f));
    materialLookup["root_temp0054_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0054_geo.ptx"), Color(0.f));
    materialLookup["root_temp0055_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0055_geo.ptx"), Color(0.f));
    materialLookup["root_temp0056_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0056_geo.ptx"), Color(0.f));
    materialLookup["root_temp0057_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/root_temp0057_geo.ptx"), Color(0.f));
    materialLookup["trunka_temp_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/trunka_temp_geo.ptx"), Color(0.f));
    materialLookup["trunkb_temp_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPandanusA/Color/trunkb_temp_geo.ptx"), Color(0.f));

    materialLookup["trunk0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/trunk0001_geo.ptx"), Color(0.f));
    materialLookup["roots_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/roots_geo.ptx"), Color(0.f));
    materialLookup["skirt_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/skirt_geo.ptx"), Color(0.f));
    materialLookup["deadstrand0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/deadstrand0001_geo.ptx"), Color(0.f));
    materialLookup["deadstrand0004_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/deadstrand0004_geo.ptx"), Color(0.f));
    materialLookup["deadstrand0003_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/deadstrand0003_geo.ptx"), Color(0.f));
    materialLookup["deadstrand0005_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/deadstrand0005_geo.ptx"), Color(0.f));
    materialLookup["deadstrand0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/deadstrand0002_geo.ptx"), Color(0.f));
    materialLookup["sheathe0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathe0001_geo.ptx"), Color(0.f));
    materialLookup["sheatha0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheatha0002_geo.ptx"), Color(0.f));
    materialLookup["sheathg0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathg0001_geo.ptx"), Color(0.f));
    materialLookup["sheathf0004_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathf0004_geo.ptx"), Color(0.f));
    materialLookup["sheathb0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathb0001_geo.ptx"), Color(0.f));
    materialLookup["sheatha0003_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheatha0003_geo.ptx"), Color(0.f));
    materialLookup["sheathc0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathc0002_geo.ptx"), Color(0.f));
    materialLookup["sheathh0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathh0002_geo.ptx"), Color(0.f));
    materialLookup["sheathc0005_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathc0005_geo.ptx"), Color(0.f));
    materialLookup["sheathc0004_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathc0004_geo.ptx"), Color(0.f));
    materialLookup["sheatha0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheatha0001_geo.ptx"), Color(0.f));
    materialLookup["sheathd0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathd0002_geo.ptx"), Color(0.f));
    materialLookup["sheathf0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathf0001_geo.ptx"), Color(0.f));
    materialLookup["sheathc0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathc0001_geo.ptx"), Color(0.f));
    materialLookup["sheathe0004_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathe0004_geo.ptx"), Color(0.f));
    materialLookup["sheathb0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathb0002_geo.ptx"), Color(0.f));
    materialLookup["sheathi0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathi0001_geo.ptx"), Color(0.f));
    materialLookup["sheathe0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathe0002_geo.ptx"), Color(0.f));
    materialLookup["sheathb0003_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathb0003_geo.ptx"), Color(0.f));
    materialLookup["sheathg0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathg0002_geo.ptx"), Color(0.f));
    materialLookup["sheathh0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathh0001_geo.ptx"), Color(0.f));
    materialLookup["sheathd0001_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathd0001_geo.ptx"), Color(0.f));
    materialLookup["sheathf0002_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathf0002_geo.ptx"), Color(0.f));
    materialLookup["sheathf0005_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathf0005_geo.ptx"), Color(0.f));
    materialLookup["sheathe0005_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathe0005_geo.ptx"), Color(0.f));
    materialLookup["sheathd0003_geo"] = std::make_shared<Lambertian>(std::make_shared<PtexLocal>("/home/cjh/workpad/moana/island/textures/isPalmRig/Color/sheathd0003_geo.ptx"), Color(0.f));

    ObjParser objParser(
        objFile,
        transform,
        false,
        Handedness::Right,
        rtcCurrentScene,
        materialLookup
    );
    auto objSurfaces = objParser.parse();

    auto bsdfJson = objJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson, materialLookup));

    std::shared_ptr<Medium> mediumPtr(nullptr);
    std::string mediumKey;
    if (checkString(objJson["internal_medium"], &mediumKey)) {
        mediumPtr = media[mediumKey];
    }

    for (auto surfacePtr : objSurfaces) {
        auto shape = surfacePtr->getShape();
        if (materialPtr) {
            auto surface = std::make_shared<Surface>(shape, materialPtr, mediumPtr);
            localSurfaces.push_back(surface);
        } else {
            localSurfaces.push_back(surfacePtr);
        }
    }

    rtcManager.registerSurfaces(rtcCurrentScene, localSurfaces);
}

static void parsePLY(
    json plyJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MaterialMap &materialLookup,
    MediaMap &media
) {
    std::ifstream plyFile(plyJson["filename"].get<std::string>());

    auto transformJson = plyJson["transform"];
    Transform transform;
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    PLYParser plyParser(plyFile, transform, false, Handedness::Left);
    auto plySurfaces = plyParser.parse();

    auto bsdfJson = plyJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson, materialLookup));

    std::shared_ptr<Medium> mediumPtr(nullptr);
    std::string mediumKey;
    if (checkString(plyJson["internal_medium"], &mediumKey)) {
        mediumPtr = media[mediumKey];
    }

    for (auto surfacePtr : plySurfaces) {
        auto shape = surfacePtr->getShape();
        if (materialPtr) {
            auto surface = std::make_shared<Surface>(shape, materialPtr, mediumPtr);
            surfaces.push_back(surface);
        } else {
            surfaces.push_back(surfacePtr);
        }
    }
}

static void parseCurve(
    json curveJson,
    std::vector<std::shared_ptr<Surface> > &surfaces,
    MaterialMap &materialLookup
) {
    std::ifstream curveFile(curveJson["filename"].get<std::string>());

    auto transformJson = curveJson["transform"];
    Transform transform;
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    CurveParser curveParser(curveFile, transform, false, Handedness::Left);
    auto curveSurfaces = curveParser.parse();

    auto bsdfJson = curveJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson, materialLookup));

    for (auto surfacePtr : curveSurfaces) {
        auto shape = surfacePtr->getShape();
        if (materialPtr) {
            auto surface = std::make_shared<Surface>(shape, materialPtr, nullptr);
            surfaces.push_back(surface);
        } else {
            surfaces.push_back(surfacePtr);
        }
    }
}

static void parseBSpline(
    json splineJson,
    RTCScene rtcCurrentScene,
    MaterialMap &materialLookup,
    RTCManager &rtcManager
) {
    std::ifstream splineFile(splineJson["filename"].get<std::string>());

    auto transformJson = splineJson["transform"];
    Transform transform;
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }

    auto bsdfJson = splineJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson, materialLookup));

    BSplineParser splineParser(splineFile, materialPtr, transform, false, Handedness::Left);
    auto splineSurfaces = splineParser.parse(
        rtcCurrentScene,
        parseFloat(splineJson["width0"]) / 2.f,
        parseFloat(splineJson["width1"]) / 2.f
    );

    for (auto localSurfaces : splineSurfaces) {
        rtcManager.registerSurfaces(
            rtcCurrentScene,
            localSurfaces
        );
    }
}

static void parseInstanced(
    json instanceJson,
    RTCScene rtcCurrentScene,
    InstanceMap &instanceLookup,
    RTCManager &rtcManager
) {
    RTCScene rtcParentScene = instanceLookup[parseString(instanceJson["instance_name"])];

    RTCGeometry rtcGeometry = rtcNewGeometry(g_rtcDevice, RTC_GEOMETRY_TYPE_INSTANCE);
    rtcSetGeometryInstancedScene(rtcGeometry, rtcParentScene);
    int rtcGeometryID = rtcAttachGeometry(rtcCurrentScene, rtcGeometry);

    const float transform[16] = {
        parseFloat(instanceJson["transform"][0]),
        parseFloat(instanceJson["transform"][1]),
        parseFloat(instanceJson["transform"][2]),
        parseFloat(instanceJson["transform"][3]),
        parseFloat(instanceJson["transform"][4]),
        parseFloat(instanceJson["transform"][5]),
        parseFloat(instanceJson["transform"][6]),
        parseFloat(instanceJson["transform"][7]),
        parseFloat(instanceJson["transform"][8]),
        parseFloat(instanceJson["transform"][9]),
        parseFloat(instanceJson["transform"][10]),
        parseFloat(instanceJson["transform"][11]),
        parseFloat(instanceJson["transform"][12]),
        parseFloat(instanceJson["transform"][13]),
        parseFloat(instanceJson["transform"][14]),
        parseFloat(instanceJson["transform"][15]),
    };

    rtcSetGeometryTransform(rtcGeometry, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, &transform[0]);
    rtcSetGeometryTimeStepCount(rtcGeometry, 1);

    rtcCommitGeometry(rtcGeometry);

    auto fakeVector = std::vector<std::shared_ptr<Surface> >();
    rtcManager.registerInstancedSurfaces(
        rtcCurrentScene,
        rtcParentScene,
        rtcGeometryID,
        fakeVector
    );
}

static void parseSphere(
    json sphereJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MaterialMap &materialLookup,
    MediaMap &media
) {
    auto bsdfJson = sphereJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson, materialLookup));

    std::shared_ptr<Medium> mediumPtr(nullptr);
    std::string mediumKey;
    if (checkString(sphereJson["internal_medium"], &mediumKey)) {
        mediumPtr = media[mediumKey];
    }

    auto sphere = std::make_shared<Sphere>(
        parsePoint(sphereJson["center"]),
        parseFloat(sphereJson["radius"])
    );

    auto surface = std::make_shared<Surface>(sphere, materialPtr, mediumPtr);

    surfaces.push_back(surface);

    Transform transform;
    auto transformJson = sphereJson["transform"];
    if (transformJson.is_object()) {
        transform = parseTransform(transformJson);
    }
    sphere->create(transform, materialPtr);
}

static void parseQuad(
    json quadJson,
    std::vector<std::shared_ptr<Surface>> &surfaces,
    MaterialMap &materialLookup
) {
    auto bsdfJson = quadJson["bsdf"];
    std::shared_ptr<Material> materialPtr(parseMaterial(bsdfJson, materialLookup));

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

static void parseMaterials(
    json materialsJson,
    MaterialMap &materialLookup
) {
    if (!materialsJson.is_array()) {
        return;
    }

    for (auto &materialJson : materialsJson) {
        std::string materialName = parseString(materialJson["name"]);
        std::shared_ptr<Material> materialPtr = parseMaterial(materialJson, materialLookup);
        materialLookup[materialName] = materialPtr;
    }
}

static std::shared_ptr<Material> parseMaterial(json bsdfJson, MaterialMap &materialLookup)
{
    if (!bsdfJson.is_object()) {
        return std::shared_ptr<Material>(nullptr);
    }

    if (bsdfJson["type"] == "reference") {
        return materialLookup.at(parseString(bsdfJson["name"]));
    } else if (bsdfJson["type"] == "phong") {
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

static Transform parseTransform(json transformJson, Transform defaultTransform)
{
    if (!transformJson.is_object()) { return defaultTransform; }
    return parseTransform(transformJson);
}

static Transform parseTransform(json transformJson)
{
    float matrix[4][4];
    float inverse[4][4];

    float scaleX = 1.f;
    float scaleY = 1.f;
    float scaleZ = 1.f;

    auto scale = transformJson["scale"];
    if (scale.is_array()) {
        scaleX = parseFloat(scale[0]);
        scaleY = parseFloat(scale[1]);
        scaleZ = parseFloat(scale[2]);
    }

    auto rotate = transformJson["rotate"];
    float rotateX = 0.f;
    float rotateY = 0.f;
    float rotateZ = 0.f;

    if (rotate.is_array()) {
         rotateX = parseFloat(rotate[0]) * M_PI / 180.f;
         rotateY = parseFloat(rotate[1]) * M_PI / 180.f;
         rotateZ = parseFloat(rotate[2]) * M_PI / 180.f;
    }

    auto translate = transformJson["translate"];
    float translateX = 0.f;
    float translateY = 0.f;
    float translateZ = 0.f;

    if (translate.is_array()) {
        translateX = -parseFloat(translate[0]); // todo: handedness
        translateY = parseFloat(translate[1]);
        translateZ = parseFloat(translate[2]);
    }

    // Transformation
    matrix::makeIdentity(matrix);
    matrix::scale(
        matrix,
        scaleX,
        scaleY,
        scaleZ
    );

    matrix::rotateX(matrix, rotateX);
    matrix::rotateY(matrix, rotateY);
    matrix::rotateZ(matrix, rotateZ);

    matrix::translate(
        matrix,
        translateX,
        translateY,
        translateZ
    );

    // Inverse
    matrix::makeIdentity(inverse);
    matrix::translate(
        inverse,
        -translateX,
        -translateY,
        -translateZ
    );

    matrix::rotateZ(inverse, -rotateZ);
    matrix::rotateY(inverse, -rotateY);
    matrix::rotateX(inverse, -rotateX);

    matrix::scale(
        inverse,
        1.f / scaleX,
        1.f / scaleY,
        1.f / scaleZ
    );

    return Transform(
        matrix,
        inverse
    );
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
