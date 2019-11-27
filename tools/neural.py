import os
import shutil
import time
from pathlib import Path

import click

import photon_reader
import process_raw_to_renders
import runner
import visualize
from mitsuba import run_mitsuba

scene_name = "ppg-cbox"
checkpoint_name = "20191127-ppg-cbox-2.t"
output_name = "test-cbox"

class Context:
    def __init__(self):
        self.mitsuba_path = Path(os.environ["MITSUBA_ROOT"])

        self.output_root = Path("/tmp") / output_name
        self.output_root.mkdir(exist_ok=True, parents=True)

        self.server_path = Path(os.environ["NSF_ROOT"])
        self.dropbox_path = Path(os.environ["RESEARCH_ROOT"])

        self.datasets_path = self.dropbox_path / "datasets"
        self.checkpoint_path = self.dropbox_path / "checkpoints" / checkpoint_name

    def scene_path(self, scene_file):
        return self.mitsuba_path / scene_name / scene_file

    def dataset_path(self, dataset_name):
        return self.datasets_path / dataset_name

@click.group()
def cli():
    pass

@cli.command()
@click.option("--all", is_flag=True)
@click.option("--point", type=int, nargs=2)
def pdf_compare(all, point):
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
    elif point:
        points = [point]
    else:
        points = [(20, 134)]

    for point in points:
        server_process = runner.launch_server(
            context.server_path,
            0,
            context.checkpoint_path
        )

        time.sleep(10) # make sure server starts up

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-training.xml"),
            context.output_root / "training.exr",
            [], # "-p1" gives reproducible results
            {
                "x": point[0],
                "y": point[1],
                "width": 400,
                "height": 400,
            }
        )

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-neural.xml"),
            context.output_root / "neural.exr",
            [ "-p1" ],
            {
                "x": point[0],
                "y": point[1],
                "width": 400,
                "height": 400,
            },
            verbose=True
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
                str(viz_out_path),
                str(context.checkpoint_path)
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
        plt.savefig(context.output_root / f"grid_{point[0]}_{point[1]}.png", bbox_inches='tight')
        plt.close()


        # Cleanup
        render_filename = f"render_{point[0]}_{point[1]}.exr"
        batch_filename = f"batch_{point[0]}_{point[1]}.exr"
        shutil.move(render_filename, context.output_root / render_filename)
        shutil.move(batch_filename, context.output_root / batch_filename)

        grid_filename = f"grid_{point[0]}_{point[1]}.bin"
        photons_filename = f"photons_{point[0]}_{point[1]}.bin"
        shutil.move(grid_filename, context.output_root / grid_filename)
        shutil.move(photons_filename, context.output_root / photons_filename)


# Quick 1spp comparison between neural and path
@cli.command()
@click.option("--include-gt", is_flag=True)
@click.option("--size", type=int, default=10)
def render(include_gt, size):
    width = size
    height = size

    context = Context()

    server_process = runner.launch_server(
        context.server_path,
        0,
        context.checkpoint_path
    )

    time.sleep(10) # make sure server starts up

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("scene-neural.xml"),
        context.output_root / "render.exr",
        [ "-p1" ],
        {
            "width": width,
            "height": height,
            "spp": 1,
        },
        verbose=True
    )

    server_process.join()

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("scene-path.xml"),
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
            context.scene_path("scene-path.xml"),
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

@cli.command()
@click.argument("x", type=int)
@click.argument("y", type=int)
def debug_pixel(x, y):
    context = Context()

    server_process = runner.launch_server(
        context.server_path,
        0,
        context.checkpoint_path
    )

    time.sleep(10) # make sure server starts up

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("scene-neural.xml"),
        f"render_{x}_{y}.exr",
        [ "-p1" ],
        {
            "x": x,
            "y": y,
            "width": 400,
            "height": 400,
        },
        verbose=True
    )

    server_process.join()

    grid_path = Path(f"grid_{x}_{y}.bin")
    raw_grid_from_renderer = photon_reader.read_raw_grid(grid_path, (10, 10))
    visualize.render_photon_grids(
        [raw_grid_from_renderer],
        f"grid_{x}_{y}.png"
    )

if __name__ == "__main__":
    cli()
