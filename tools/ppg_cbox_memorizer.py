import json
import shutil
import subprocess
import tempfile
from pathlib import Path

import runner
from dataset_info import DatasetInfo

def custom_pdf_json(i, pdf_samples, render_path, scene_path):
    return {
        "force": True,
        "spp": -1,

        "output_directory": str(render_path / f"iteration-{i:04d}"),
        "output_name": "--",
        "integrator": "PDFIntegrator",
        "scene": str(scene_path / f"ppg-cbox-{i:04d}.json"),
        "showUI": False,

        "startBounce": 2,
        "lastBounce": 2,

        "lightPhiSteps": 20,
        "lightThetaSteps": 20,

        "phiSteps": 10,
        "thetaSteps": 10,
        "debugSearchCount": 100,

        "pdf_samples": pdf_samples,
        "port_offset": 0,

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }

def custom_path_json(i, render_path, scene_path):
    return {
        "force": True,
        "spp": 32,

        "output_directory": str(render_path / f"iteration-{i:04d}"),
        "output_name": "--",
        "integrator": "PathTracer",
        "scene": str(scene_path / f"ppg-cbox-{i:04d}.json"),
        "showUI": False,

        "startBounce": 2,
        "lastBounce": 2,

        "lightPhiSteps": 20,
        "lightThetaSteps": 20,

        "phiSteps": 10,
        "thetaSteps": 10,
        "debugSearchCount": 100,

        "pdf_samples": -1,
        "port_offset": 0,

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }

def run(scene_count, pdf_samples, dataset_info, mode):
    for i in range(scene_count):
        one(i, dataset_info.scene_path(mode))

        if i == 0:
            path_job = custom_path_json(
                i,
                dataset_info.render_path(mode),
                dataset_info.scene_path(mode)
            )
            runner.run_renderer(path_job)

        if mode == "test": continue

        pdf_job = custom_pdf_json(
            i,
            pdf_samples,
            dataset_info.render_path(mode),
            dataset_info.scene_path(mode)
        )
        runner.run_renderer(pdf_job)

def one(i, target_path):
    obj_source_path = Path("../ppg-cbox")
    for obj_source_file in obj_source_path.glob("*.obj"):
        shutil.copy(obj_source_file, target_path)

    scene_filename = f"ppg-cbox-{i:04d}.json"

    with open(target_path / scene_filename, "w") as f:
        f.write(scene_json(target_path))

def scene_json(obj_path):
    json_content = {
        "sensor": {
            "lookAt": {
                "origin": [ "278", "273", "-800" ],
                "target": [ "278", "273", "-799" ],
                "up": [ "0", "1", "0" ]
            },
            "fov": "39.3077"
        },
        "models": [
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_luminaire.obj"),

                "transform": {
                    "rotate": [ "180", "0", "0" ],
                    "translate": [ "0", "1020", "550" ]
                },

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.0", "0.0", "0.0" ],
                    "emit": [ "17", "12", "4" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_floor.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.725", "0.71", "0.68" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_ceiling.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.725", "0.71", "0.68" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_back.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.725", "0.71", "0.68" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_greenwall.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.14", "0.45", "0.091" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_redwall.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.63", "0.065", "0.05" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_smallbox.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.725", "0.71", "0.68" ]
                }
            },
            {
                "type": "obj",
                "filename": str(obj_path / "cbox_largebox.obj"),

                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.725", "0.71", "0.68" ]
                }
            }
        ]
    }
    return json.dumps(json_content, indent=2) + "\n"

if __name__ == "__main__":
    training_scenes = 50
    test_scenes = 1
    pdf_samples = 30

    dataset_info = DatasetInfo("/home/cjh/workpad/datasets/ppg-cbox")

    run(training_scenes, pdf_samples, dataset_info, "train")
    run(test_scenes, None, dataset_info, "test")
