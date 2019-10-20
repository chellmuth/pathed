import json
import subprocess
import tempfile
from pathlib import Path

import cornell_randomizer
import runner
from dataset_info import DatasetInfo

def custom_pdf_json(i, pdf_samples, render_path, scene_path):
    return {
        "force": True,
        "spp": -1,

        "output_directory": str(render_path / f"iteration-{i:04d}"),
        "output_name": "--",
        "integrator": "PDFIntegrator",
        "scene": str(scene_path / f"cornell-{i:04d}.json"),
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
        "scene": str(scene_path / f"cornell-{i:04d}.json"),
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
        cornell_randomizer.one(i, dataset_info.scene_path(mode))

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


if __name__ == "__main__":
    training_scenes = 100
    test_scenes = 10
    pdf_samples = 50

    dataset_info = DatasetInfo("/home/cjh/workpad/cornell-dataset")
    dataset_info.check_and_create()

    run(training_scenes, pdf_samples, dataset_info, "train")
    run(test_scenes, None, dataset_info, "test")
