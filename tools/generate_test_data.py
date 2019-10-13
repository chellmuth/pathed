import json
import subprocess
import tempfile

import cornell_randomizer

def custom_pdf_json(i):
    return {
        "force": True,

        "output_directory": f"/home/cjh/workpad/cornell-dataset/iteration-{i:04d}",
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

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }

def custom_path_json(i):
    return {
        "force": True,

        "output_directory": f"/home/cjh/workpad/cornell-dataset/iteration-{i:04d}",
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

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }

def run():
    count = 100

    for i in range(count):
        cornell_randomizer.one(i)

        job_content = custom_pdf_json(i)
        with tempfile.NamedTemporaryFile("w") as f:
            f.write(json.dumps(job_content, indent=2))
            f.flush()

            try:
                output = subprocess.check_call(["./pathed", f.name], cwd="../Release")
            except subprocess.CalledProcessError:
                print("ERROR (pdf)!", i)

        job_content = custom_path_json(i)
        with tempfile.NamedTemporaryFile("w") as f:
            f.write(json.dumps(job_content, indent=2))
            f.flush()

            try:
                output = subprocess.check_call(["./pathed", f.name], cwd="../Release")
            except subprocess.CalledProcessError:
                print("ERROR! (path)", i)

    # later: gather step

if __name__ == "__main__":
    run()
