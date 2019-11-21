from pathlib import Path

import click

from mitsuba import run_mitsuba

class Context:
    def __init__(self, scene):
        self.mitsuba_path = Path("/home/cjh/src/mitsuba")
        self.scene_path = self.mitsuba_path / scene

        self.output_root = Path("/tmp/mitsuba-tests")
        self.output_root.mkdir(exist_ok=True, parents=True)

@click.group()
def cli():
    pass

@cli.command()
def pdf_compare():
    mitsuba_path = Path("/home/cjh/src/mitsuba")
    fisheye_scene_path = mitsuba_path / "cornell-box/scene-training.xml"
    neural_scene_path = mitsuba_path / "cornell-box/scene-neural.xml"

    output_root = Path("/tmp/mitsuba-tests")
    output_root.mkdir(exist_ok=True, parents=True)

    point = (246, 315)

    run_mitsuba(
        mitsuba_path,
        fisheye_scene_path,
        output_root / "training.exr",
        [ "-p1" ],
        {
            "x": point[0],
            "y": point[1],
            "width": 400,
            "height": 400,
        }
    )

    run_mitsuba(
        mitsuba_path,
        neural_scene_path,
        output_root / "neural.exr",
        [ "-p1" ],
        {
            "x": point[0],
            "y": point[1],
            "width": 400,
            "height": 400,
        }
    )

@cli.command()
def render():
    context = Context("cornell-box/scene-neural.xml")

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path,
        context.output_root / "render.exr",
        [ "-p1" ],
        {
            "width": 20,
            "height": 20,
        }
    )

if __name__ == "__main__":
    cli()
