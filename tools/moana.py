import json
import os
from pathlib import Path

import fov_converter

MoanaPath = Path(os.environ["MOANA_ROOT"])

def flip_x(xyz_vector):
    return [ xyz_vector[0] * -1, xyz_vector[1], xyz_vector[2] ]

def convert_camera(camera_json):
    pathed_json = {
        "name": camera_json["name"],
        "fov": str(fov_converter.fov_x_deg_to_fov_y_deg(camera_json["fov"], 238, 100)),
        "lookAt": {
            "origin": [ str(f) for f in flip_x(camera_json["eye"]) ],
            "target": [ str(f) for f in flip_x(camera_json["look"]) ],
            "up": [ str(f) for f in flip_x(camera_json["up"]) ],
        }
    }

    return pathed_json

def convert_cameras(cameras_directory, out_path):
    sensors_json = []

    for camera_file in cameras_directory.glob("*.json"):
        sensors_json.append(convert_camera(json.load(open(camera_file, "r"))))

    pathed_json = {
        "comment": "Generated by tools/moana.py",
        "sensors": sensors_json
    }

    with open(out_path, "w") as f:
        json.dump(pathed_json, f, indent=2)

def parse_archive(archive_json):
    instances = []
    models = []

    filename = MoanaPath / archive_json["jsonFile"]
    instances_json = json.load(open(filename, "r"))

    for archive_name in archive_json["archives"]:
        instance = {
            "type": "instance",
            "name": archive_name,
            "models": [
                {
                    "type": "obj",
                    "filename": str(MoanaPath / archive_name)
                }
            ]
        }
        instances.append(instance)


        archive_instances_json = instances_json[archive_name]
        for transform in archive_instances_json.values():
            models.append({
                "type": "instanced",
                "instance_name": archive_name,
                "transform": [ str(f) for f in transform ]
            })

    return instances, models

def parse_curve(curve_json):
    filename = MoanaPath / curve_json["jsonFile"]
    instances_json = json.load(open(filename, "r"))

    model = {
        "type": "b-spline",
        "filename": str(MoanaPath / filename),
        "width0": str(curve_json["widthRoot"]),
        "width1": str(curve_json["widthTip"]),
    }

    return [], [model]

def find_primitives(element_json):
    instances = []
    models = []

    key = "instancedPrimitiveJsonFiles"
    if key not in element_json:
        return [], []

    instanced_primitive_json = element_json["instancedPrimitiveJsonFiles"]
    for primitive_name, primitive_json in instanced_primitive_json.items():
        print("primitive name:", primitive_name)
        if primitive_json["type"] == "archive":
            next_instances, next_models = parse_archive(primitive_json)
            instances.extend(next_instances)
            models.extend(next_models)
        elif primitive_json["type"] == "curve":
            next_instances, next_models = parse_curve(primitive_json)
            instances.extend(next_instances)
            models.extend(next_models)

    return instances, models

def parse_instances(instanced_copies_json, instance_name):
    instances_json = []
    elements_json = []
    for instanced_copy_json in instanced_copies_json.values():
        if "geomObjFile" in instanced_copy_json:
            element_json = parse_element(instanced_copy_json)
            elements_json.extend(element_json)
            continue

        instances_json.append({
            "type": "instanced",
            "instance_name": instance_name,
            "transform": [ str(f) for f in instanced_copy_json["transformMatrix"] ]
        })

    return instances_json, elements_json

def parse_element(element_json):
    leaf, leaves = find_primitives(element_json)

    instance_json = {
        "type": "instance",
        "name": element_json["name"],
        "models": [
            {
                "type": "obj",
                "filename": str(MoanaPath / element_json["geomObjFile"])
            },
            *leaves
        ]
    }

    instances_json, elements_json = parse_instances(
        element_json.get("instancedCopies", {}),
        element_json["name"]
    )
    instances_json.append(
        {
            "type": "instanced",
            "instance_name": element_json["name"],
            "transform": [ str(f) for f in element_json["transformMatrix"] ]
        }
    )

    return leaf + [instance_json] + instances_json + elements_json

def convert_element(element_json, out_path):
    pathed_json = {
        "models": parse_element(element_json)
    }

    with open(out_path, "w") as f:
        json.dump(pathed_json, f, indent=2)


def convert_elements(elements_directory, out_directory, whitelist=None):
    for element_directory in elements_directory.glob("is*"):
        if whitelist and element_directory.name not in whitelist: continue
        element_path = element_directory / f"{element_directory.name}.json"
        out_path = out_directory / f"{element_directory.name}.json"
        convert_element(json.load(open(element_path, "r")), out_path)

def generate_moana_config(camera_name, element_names, assets_directory, out_path):
    pathed_json = {
        "sensor": None,

        "models": [],

        "environmentLight": {
            "filename": "background.exr",
            "scale": "1"
        }
    }

    sensors_json = json.load(open(assets_directory / "sensors.json", "r"))
    camera_json = [
        camera_json
        for camera_json
        in sensors_json["sensors"]
        if camera_json["name"] == camera_name
    ][0]

    pathed_json["sensor"] = camera_json

    for element_name in element_names:
        element_json = json.load(open(assets_directory / f"{element_name}.json", "r"))

        pathed_json["models"].extend(element_json["models"])

    with open(out_path, "w") as f:
        json.dump(pathed_json, f, indent=2)

if __name__ == "__main__":
    convert_cameras(
        MoanaPath / "json/cameras",
        Path("../moana/sensors.json")
    )

    elements = [
        # "isBayCedarA1",
        # "isDunesA",
        # "isHibiscusYoung",
        # "isLavaRocks",
        # "isPalmDead",
        # "isBeach",
        # "isDunesB",
        # "isMountainA",
        "isPalmRig",
        # "isCoastline",
        # "isGardeniaA",
        # "isMountainB",
        "isPandanusA",
        # "isCoral",
        "isHibiscus",
        "isKava",
        # "isNaupakaA",
        # "isIronwoodA1",
        # "isIronwoodB"
    ]
    convert_elements(
        MoanaPath / "json",
        Path("../moana/"),
        whitelist=elements
    )

    generate_moana_config(
        "shotCam",
        elements,
        Path("../moana"),
        Path("../moana.json")
    )
