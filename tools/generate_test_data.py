import json
import subprocess
import tempfile
from pathlib import Path

import cornell_randomizer
import runner

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

        if mode == "test": continue

        pdf_job = custom_pdf_json(
            i,
            pdf_samples,
            dataset_info.render_path(mode),
            dataset_info.scene_path(mode)
        )
        runner.run_renderer(pdf_job)

        path_job = custom_path_json(
            i,
            dataset_info.render_path(mode),
            dataset_info.scene_path(mode)
        )
        runner.run_renderer(path_job)

class DatasetInfo:
    def __init__(self, root):
        self.root = Path(root)

        self.train = self.root / "train"
        self.test = self.root / "test"

        self.train_scenes = self.train / "scenes"
        self.test_scenes = self.test / "scenes"

        self.train_renders = self.train / "renders"
        self.test_renders = self.test / "renders"

    def check_and_create(self):
        directories = [
            self.root,
            self.train,
            self.test,
            self.train_scenes,
            self.test_scenes,
            self.train_renders,
            self.test_renders
        ]

        for directory in directories:
            directory.mkdir(exist_ok=True)

    def scene_path(self, mode):
        if mode == "train":
            return self.train_scenes
        if mode == "test":
            return self.test_scenes

        raise ValueError("Unsupported mode")

    def render_path(self, mode):
        if mode == "train":
            return self.train_renders
        if mode == "test":
            return self.test_renders

        raise ValueError("Unsupported mode")

if __name__ == "__main__":
    training_scenes = 100
    test_scenes = 10
    pdf_samples = 50

    dataset_info = DatasetInfo("/home/cjh/workpad/cornell-dataset")
    dataset_info.check_and_create()

    run(training_scenes, pdf_samples, dataset_info, "train")
    run(test_scenes, None, dataset_info, "test")
