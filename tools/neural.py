import os
import time
from pathlib import Path

import click

import runner
from mitsuba import run_mitsuba

class Context:
    def __init__(self):
        self.mitsuba_path = Path(os.environ["MITSUBA_ROOT"])

        self.output_root = Path("/tmp/mitsuba-tests")
        self.output_root.mkdir(exist_ok=True, parents=True)

        self.server_path = Path(os.environ["NSF_ROOT"])
        self.dropbox_path = Path(os.environ["DROPBOX_ROOT"])
        self.checkpoint_path = self.dropbox_path / "checkpoints/20191120-mitsuba-1.t"

    def scene_path(self, scene):
        return self.mitsuba_path / scene


@click.group()
def cli():
    pass

@cli.command()
def pdf_compare():
    context = Context()

    point = (246, 315)

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

@cli.command()
def render():
    width = 20
    height = 20

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
        ["-p1"],
        {
            "width": width,
            "height": height,
        }
    )

if __name__ == "__main__":
    cli()
