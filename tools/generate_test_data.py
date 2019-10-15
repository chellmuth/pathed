import json
import subprocess
import tempfile
from pathlib import Path

import cornell_randomizer

def custom_pdf_json(i, dataset_path):
    return {
        "force": True,
        "spp": -1,

        "output_directory": str(dataset_path / f"iteration-{i:04d}"),
        "output_name": "--",
        "integrator": "PDFIntegrator",
        "scene": f"procedural/cornell-{i:04d}.json",
        "showUI": False,

        "startBounce": 2,
        "lastBounce": 2,

        "lightPhiSteps": 20,
        "lightThetaSteps": 20,

        "phiSteps": 10,
        "thetaSteps": 10,
        "debugSearchCount": 100,

        "portOffset": 0,

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }

def custom_path_json(i, dataset_path):
    return {
        "force": True,
        "spp": 32,

        "output_directory": str(dataset_path / f"iteration-{i:04d}"),
        "output_name": "--",
        "integrator": "PathTracer",
        "scene": f"procedural/cornell-{i:04d}.json",
        "showUI": False,

        "startBounce": 2,
        "lastBounce": 2,

        "lightPhiSteps": 20,
        "lightThetaSteps": 20,

        "phiSteps": 10,
        "thetaSteps": 10,
        "debugSearchCount": 100,

        "portOffset": 0,

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }

def run(dir_name, scene_count, dataset_path, test_only=False):
    for i in range(scene_count):
        cornell_randomizer.one(i, dir_name)

        if test_only: continue

        job_content = custom_pdf_json(i, dataset_path)
        with tempfile.NamedTemporaryFile("w") as f:
            f.write(json.dumps(job_content, indent=2))
            f.flush()

            try:
                output = subprocess.check_call(["./pathed", f.name], cwd="../Release")
            except subprocess.CalledProcessError:
                print("ERROR (pdf)!", i)

        job_content = custom_path_json(i, dataset_path)
        with tempfile.NamedTemporaryFile("w") as f:
            f.write(json.dumps(job_content, indent=2))
            f.flush()

            try:
                output = subprocess.check_call(["./pathed", f.name], cwd="../Release")
            except subprocess.CalledProcessError:
                print("ERROR! (path)", i)

if __name__ == "__main__":
    dataset_path = Path("/home/cjh/workpad/cornell-dataset")

    run("procedural", 100, dataset_path, test_only=False)
    run("procedural-test", 10, dataset_path, test_only=True)
