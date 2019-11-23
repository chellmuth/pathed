import os
import time
from pathlib import Path

import click

import photon_reader
import process_raw_to_renders
import runner
import visualize
from mitsuba import run_mitsuba

class Context:
    def __init__(self):
        self.mitsuba_path = Path(os.environ["MITSUBA_ROOT"])

        self.output_root = Path("/tmp/test-mitsuba")
        self.output_root.mkdir(exist_ok=True, parents=True)

        self.server_path = Path(os.environ["NSF_ROOT"])
        self.dropbox_path = Path(os.environ["DROPBOX_ROOT"])

        self.datasets_path = self.dropbox_path / "datasets"
        self.checkpoint_path = self.dropbox_path / "checkpoints/20191123-mitsuba-3.t"

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
        points = [(20, 134)]

    for point in points:
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("cornell-box/scene-training.xml"),
            context.output_root / "training.exr",
            [], # "-p1" gives reproducible results
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

        grid_path = Path(f"grid_{point[0]}_{point[1]}.bin")
        grid = photon_reader.read_raw_grid(grid_path, (10, 10))

        from matplotlib import cm, pyplot as plt

        figure, axes = plt.subplots(1, 1)
        visualize.photon_bundle(axes, grid)

        plt.tight_layout()
        plt.savefig(f"grid_{point[0]}_{point[1]}.png", bbox_inches='tight')
        plt.close()


# Quick 1spp comparison between neural and path
@cli.command()
@click.option("--include-gt", is_flag=True)
def render(include_gt):
    width = 80
    height = 80

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

    if include_gt:
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

# Convert files generated by Mitsuba into the original intermediate format
@cli.command()
@click.argument("dataset_name")
def process(dataset_name):
    context = Context()
    dataset_path = context.dataset_path(dataset_name)

    process_raw_to_renders.run(
        500,
        dataset_path / "raw",
        dataset_path / "train/renders",
    )

# Compare final grids between:
# * Photon digest on renderer + python splatting
# * Direct renderer splatting
@cli.command()
@click.argument("bundle_path", type=Path)
@click.argument("grid_path", type=Path)
def compare_photons(bundle_path, grid_path):
    grid = photon_reader.build_grid(bundle_path, (10, 10))
    raw_grid_from_bundle = grid.raw_grid()
    raw_grid_from_renderer = photon_reader.read_raw_grid(grid_path, (10, 10))

    visualize.render_photon_grids(
        [raw_grid_from_bundle, raw_grid_from_renderer],
        "photon-comparison.png"
    )

if __name__ == "__main__":
    cli()
