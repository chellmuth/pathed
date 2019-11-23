import os
import time
from pathlib import Path

import click

import process_raw_to_renders
import runner
import visualize
from mitsuba import run_mitsuba

class Context:
    def __init__(self):
        self.mitsuba_path = Path(os.environ["MITSUBA_ROOT"])

        self.output_root = Path("/tmp/mitsuba-tests")
        self.output_root.mkdir(exist_ok=True, parents=True)

        self.server_path = Path(os.environ["NSF_ROOT"])
        self.dropbox_path = Path(os.environ["DROPBOX_ROOT"])

        self.datasets_path = self.dropbox_path / "datasets"
        self.checkpoint_path = self.dropbox_path / "checkpoints/20191122-mitsuba-2.t"

    def scene_path(self, scene):
        return self.mitsuba_path / scene

    def dataset_path(self, dataset_name):
        return self.datasets_path / dataset_name

@click.group()
def cli():
    pass

@cli.command()
@click.option("--all", is_flag=True)
def pdf_compare(all):
    context = Context()

    if all:
        points = [
            (20, 134), # left wall
            (268, 151), # back wall
            (384, 240), # right wall
            (114, 26), # ceiling
            (313, 349), # short box - right side
            (246, 315), # short box - front side
            (228, 270), # short box - top side
            (82, 388), # floor
            (146, 210), # tall box - front side
            (94, 175), # tall box - left side
        ]
    else:
        points = [(246, 315)]

    for point in points:
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("cornell-box/scene-training.xml"),
            context.output_root / "training.exr",
            [ "-p1" ],
            {
                "x": point[0],
                "y": point[1],
                "width": 400,
                "height": 400,
            }
        )

        server_process = runner.launch_server(
            context.server_path,
            0,
            context.checkpoint_path
        )

        time.sleep(10) # make sure server starts up

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("cornell-box/scene-neural.xml"),
            context.output_root / "neural.exr",
            [ "-p1" ],
            {
                "x": point[0],
                "y": point[1],
                "width": 400,
                "height": 400,
            }
        )

        server_process.join()

        pdf_out_path = context.output_root / f"pdf_{point[0]}_{point[1]}.exr"
        photons_out_path = context.output_root / f"photon-bundle_{point[0]}_{point[1]}.dat"
        process_raw_to_renders.execute(
            Path(f"render_{point[0]}_{point[1]}.exr"),
            Path(f"photons_{point[0]}_{point[1]}.bin"),
            pdf_out_path,
            photons_out_path
        )

        # run viz
        viz_out_path = context.output_root / f"viz_{point[0]}_{point[1]}.png"

        runner.run_nsf_command(
            context.server_path,
            "evaluate.py",
            [
                str(pdf_out_path),
                str(photons_out_path),
                str(viz_out_path)
            ]
        )

        batch_path = Path(f"batch_{point[0]}_{point[1]}.exr")
        neural_out_path = context.output_root / f"neural_{point[0]}_{point[1]}.png"
        visualize.convert_to_density_mesh(
            batch_path,
            neural_out_path
        )

@cli.command()
def render():
    width = 400
    height = 400

    context = Context()

    server_process = runner.launch_server(
        context.server_path,
        0,
        context.checkpoint_path
    )

    time.sleep(10) # make sure server starts up

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("cornell-box/scene-neural.xml"),
        context.output_root / "render.exr",
        [ "-p1" ],
        {
            "width": width,
            "height": height,
        }
    )

    server_process.join()

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("cornell-box/scene-path.xml"),
        context.output_root / "path.exr",
        [],
        {
            "width": width,
            "height": height,
            "spp": 1,
        }
    )

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("cornell-box/scene-path.xml"),
        context.output_root / "gt.exr",
        [],
        {
            "width": width,
            "height": height,
            "spp": 2**12,
        }
    )

@cli.command()
@click.argument("dataset_name")
def process(dataset_name):
    context = Context()
    dataset_path = context.dataset_path(dataset_name)

    process_raw_to_renders.run(
        dataset_path / "raw",
        dataset_path / "train/renders",
    )

if __name__ == "__main__":
    cli()
