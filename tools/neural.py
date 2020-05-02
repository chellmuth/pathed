import glob
import os
import re
import shutil
import time
from collections import namedtuple
from pathlib import Path

import click
import pyexr

import compare_pdfs
import photon_reader
import process_raw_to_renders
import simple_chart
import runner
import variance
import visualize
from mitsuba import run_mitsuba
from parameters import GridShape

default_scene_name = "staircase"
default_output_name = "staircase"

default_checkpoints = {
    "kitchen": None,
    "cbox-ppg": "20191205-cbox-ppg-2",
    "cbox-bw": "20191217-cbox-bw-4",
}

dimensions = {
    "kitchen": (1280, 720),
    "kitchen-diffuse": (80, 45),
    "cbox-ppg": (400, 400),
    "cbox-bw": (400, 400),
    "green-bounce": (80, 45),
    "staircase": (45, 80),
}

interesting_points = [
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

Artifacts = namedtuple("Artifacts", [ "render_path", "batch_path", "samples_path", "server_viz_path" ])

def build_output_root(root_path, output_name, comment, reuse):
    counter = 1
    for file_name in glob.glob(str(root_path / output_name) + "-[0-9]*"):
        match = re.search(output_name + r"-(\d+)", file_name)
        if match:
            identifier = int(match.group(1))

            if reuse:
                counter = max(counter, identifier)
            else:
                counter = max(counter, identifier + 1)

    dir_name = output_name + f"-{counter}"
    if comment:
        dir_name += f"--{comment}"

    return Path(root_path / dir_name)

def get_default_checkpoint_stem(scene_name, root_path, verbose=False):
    checkpoint_files = glob.glob(str(root_path / scene_name) + "*")
    if checkpoint_files:
        latest_checkpoint_path = Path(sorted(checkpoint_files)[-1])
        if verbose:
            print(f"Using latest checkpoint: {latest_checkpoint_path}")

        return latest_checkpoint_path.stem

    if verbose:
        print(f"Using default checkpoint: {default_checkpoints[self.scene_name]}")

    return default_checkpoints[self.scene_name]

class Context:
    def __init__(self, scene_name=None, checkpoint_name=None, output_name=None, comment=None, reuse_output_directory=False):
        self.scene_name = scene_name or default_scene_name

        self.mitsuba_path = Path(os.environ["MITSUBA_ROOT"])

        self.output_root = build_output_root(
            Path("/tmp"),
            output_name or default_output_name,
            comment,
            reuse_output_directory
        )
        self.output_root.mkdir(exist_ok=True, parents=True)

        self.server_path = Path(os.environ["NSF_ROOT"])
        self.research_path = Path(os.environ["RESEARCH_ROOT"])

        self.scenes_path = self.research_path / "scenes" / self.scene_name
        self.datasets_path = self.research_path / "datasets"

        self.checkpoint_root = self.research_path / "checkpoints"
        self.checkpoint_name = checkpoint_name or get_default_checkpoint_stem(self.scene_name, self.checkpoint_root, verbose=True)
        self.checkpoint_path = self.checkpoint_root / f"{self.checkpoint_name}.t"

        self.gt_path = self.scenes_path / "gt.exr"

    def scene_path(self, scene_file):
        return self.scenes_path / scene_file

    def dataset_path(self, dataset_name):
        return self.datasets_path / dataset_name

    def artifacts(self, point):
        return Artifacts(
            render_path=self.output_root / f"render_{point[0]}_{point[1]}.exr",
            batch_path=self.output_root / f"batch_{point[0]}_{point[1]}.exr",
            samples_path=self.output_root / f"samples_{point[0]}_{point[1]}.bin",
            server_viz_path=self.server_path / f"server_viz_{point[0]}_{point[1]}.png",
        )

    def gt_pixel(self, point, channel=0):
        exr = pyexr.read(str(self.gt_path))
        return exr[point[1]][point[0]][channel]

@click.group()
def cli():
    pass

@cli.command()
@click.option("--all", is_flag=True)
@click.option("--point", type=int, nargs=2)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
def pdf_compare(all, point, output_name, comment, reuse):
    context = Context(output_name=output_name, comment=comment, reuse_output_directory=reuse)

    if all:
        points = interesting_points
    elif point:
        points = [point]
    else:
        points = [(20, 134)]

    print(f"Processing points: {points}")

    for point in points:
        server_viz_path = context.artifacts(point).server_viz_path
        server_process = runner.launch_server(
            context.server_path,
            0,
            context.checkpoint_path,
            server_viz_path
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
                "width": dimensions[default_scene_name][0],
                "height": dimensions[default_scene_name][1],
            },
            verbose=True
        )

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-neural.xml"),
            context.output_root / "neural.exr",
            [ "-p1" ],
            {
                "x": point[0],
                "y": point[1],
                "width": dimensions[default_scene_name][0],
                "height": dimensions[default_scene_name][1],
            },
            verbose=True
        )

        server_process.join()

        server_process = runner.launch_server(
            context.server_path,
            0,
            context.checkpoint_path
        )

        time.sleep(10) # make sure server starts up

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-neural.xml"),
            context.output_root / "neural.exr",
            [ "-p1" ],
            {
                "x": point[0],
                "y": point[1],
                "spp": 1024,
                "width": dimensions[default_scene_name][0],
                "height": dimensions[default_scene_name][1],
            },
            verbose=True
        )

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
        grid = photon_reader.read_raw_grid(grid_path, GridShape)

        from matplotlib import cm, pyplot as plt

        figure, axes = plt.subplots(1, 1)
        visualize.photon_bundle(axes, grid)

        plt.tight_layout()
        plt.savefig(context.output_root / f"grid_{point[0]}_{point[1]}.png", bbox_inches='tight')
        plt.close()

        # Cleanup
        render_filename = f"render_{point[0]}_{point[1]}.exr"
        batch_filename = f"batch_{point[0]}_{point[1]}.exr"
        samples_filename = f"samples_{point[0]}_{point[1]}.bin"

        shutil.move(render_filename, context.output_root / render_filename)
        shutil.move(batch_filename, context.output_root / batch_filename)

        grid_filename = f"grid_{point[0]}_{point[1]}.bin"
        photons_filename = f"photons_{point[0]}_{point[1]}.bin"
        neural_filename = f"neural_{point[0]}_{point[1]}.bin"
        shutil.move(grid_filename, context.output_root / grid_filename)
        shutil.move(photons_filename, context.output_root / photons_filename)
        shutil.move(neural_filename, context.output_root / neural_filename)
        shutil.move(samples_filename, context.output_root / samples_filename)
        shutil.move(server_viz_path, context.output_root / server_viz_path.name)

@cli.command()
@click.option("--all", is_flag=True)
@click.option("--point", type=int, nargs=2)
def pdf_analyze(all, point):
    context = Context()

    if all:
        points = interesting_points
    elif point:
        points = [point]
    else:
        points = [(20, 134)]

    divergence_list = []
    error_list = []
    for point in points:
        artifacts = context.artifacts(point)
        render_path = artifacts.render_path
        batch_path = artifacts.batch_path
        samples_path = artifacts.samples_path

        divergences = compare_pdfs.run(render_path, batch_path)

        samples = variance.read_bin(samples_path)
        errors = variance.errors(samples, 4, context.gt_pixel(point))

        divergence_list.append(divergences)
        error_list.append(errors)

        compare_pdfs.generate_zero_map(render_path, batch_path, f"zero_{point[0]}_{point[1]}.exr")

        print(point)
        print(divergences)
        print(errors)

    # simple_chart.scatter(
    #     [ d["kl"] for d in divergence_list ],
    #     [ e["variance"] for e in error_list ],
    #     title="Correlation between KL divergence and variance",
    #     x_label="KL divergence",
    #     y_label="Variance"
    # )

    # simple_chart.scatter(
    #     [ d["chi2"] for d in divergence_list ],
    #     [ e["variance"] for e in error_list ],
    #     title="Correlation between Chi-squared divergence and variance",
    #     x_label="Chi-squared",
    #     y_label="Variance"
    # )

    # simple_chart.scatter(
    #     [ d["abs"] for d in divergence_list ],
    #     [ e["variance"] for e in error_list ],
    #     title="Correlation between squared pdf difference and variance",
    #     x_label="Squared pdf distance",
    #     y_label="Variance"
    # )

    # simple_chart.scatter(
    #     [ d["kl"] for d in divergence_list ],
    #     [ e["mrse"] for e in error_list ],
    #     title="Correlation between KL divergence and MrSE",
    #     x_label="KL divergence",
    #     y_label="MrSE"
    # )

    # simple_chart.scatter(
    #     [ d["chi2"] for d in divergence_list ],
    #     [ e["mrse"] for e in error_list ],
    #     title="Correlation between Chi-squared divergence and MrSE",
    #     x_label="Chi-squared",
    #     y_label="MrSE"
    # )

    # simple_chart.scatter(
    #     [ d["squared"] for d in divergence_list ],
    #     [ e["mrse"] for e in error_list ],
    #     title="Correlation between squared pdf difference and MrSE",
    #     x_label="Squared pdf distance",
    #     y_label="MrSE"
    # )

    simple_chart.scatter(
        [ d["chi2"] for d in divergence_list ],
        [ e["bias"] for e in error_list ],
        title="Correlation between Chi-squared divergence and bias",
        x_label="Chi-squared",
        y_label="Bias"
    )

# Quick 1spp comparison between neural and path
@cli.command()
@click.option("--skip-neural", is_flag=True)
@click.option("--skip-path", is_flag=True)
@click.option("--include-gt", is_flag=True)
@click.option("--size", type=int, default=10)
@click.option("--spp", type=int, default=1)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
def render(skip_neural, skip_path, include_gt, size, spp, comment, reuse):
    context = Context(comment=comment, reuse_output_directory=reuse)
    _render(context, skip_neural, skip_path, include_gt, size, spp)

def _render(context, skip_neural, skip_path, include_gt, size, spp):
    scene_name = context.scene_name
    aspect_ratio = dimensions[scene_name][1] / dimensions[scene_name][0]

    width = size
    height = int(size * aspect_ratio)

    if not skip_neural:
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
                "spp": spp,
            },
            verbose=True
        )

        server_process.join()

    if not skip_path:
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-path.xml"),
            context.output_root / "path.exr",
            [],
            {
                "width": width,
                "height": height,
                "spp": spp,
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
@click.option("--bundle-path", type=Path, multiple=True)
@click.option("--grid-path", type=Path, multiple=True)
def compare_photons(bundle_path, grid_path):
    raw_grids = []

    for path in bundle_path:
        grid = photon_reader.build_grid(path, (10, 10))
        raw_grids.append(grid.raw_grid())

    for path in grid_path:
        raw_grids.append(photon_reader.read_raw_grid(path, (10, 10)))

    visualize.render_photon_grids(
        raw_grids,
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
            "width": dimensions[default_scene_name][0],
            "height": dimensions[default_scene_name][1],
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

@cli.command()
@click.argument("x", type=int)
@click.argument("y", type=int)
def walk_pixel(x, y):
    context = Context()

    scale = 5

    for offset in range(50):
        x_scaled = scale * x + offset
        y_scaled = scale * y

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
                "x": x_scaled,
                "y": y_scaled,
                "width": dimensions[default_scene_name][0] * scale,
                "height": dimensions[default_scene_name][1] * scale,
            },
            verbose=True
        )

        server_process.join()

        batch_path = Path(f"batch_{x_scaled}_{y_scaled}.exr")
        neural_out_path = context.output_root / f"neural_{x}_{y}__{offset}.png"
        visualize.convert_to_density_mesh(
            batch_path,
            neural_out_path
        )

        batch_filename = f"batch_{x_scaled}_{y_scaled}.exr"
        grid_filename = f"grid_{x_scaled}_{y_scaled}.bin"
        neural_filename = f"neural_{x_scaled}_{y_scaled}.bin"

        os.remove(batch_filename)
        os.remove(grid_filename)
        os.remove(neural_filename)

    render_filename = f"render_{x}_{y}.exr"
    os.remove(render_filename)

@cli.command()
@click.argument("x", type=int)
@click.argument("y", type=int)
def check_convergence(x, y):
    context = Context()

    point = [x, y]
    server_process = runner.launch_server(
        context.server_path,
        0,
        context.checkpoint_path
    )

    time.sleep(10) # make sure server starts up

    power = 10

    run_mitsuba(
        context.mitsuba_path,
        context.scene_path("scene-neural.xml"),
        context.output_root / "neural.exr",
        [ "-p1" ],
        {
            "x": point[0],
            "y": point[1],
            "spp": 2 ** power,
            "width": dimensions[default_scene_name][0],
            "height": dimensions[default_scene_name][1],
        },
        verbose=True
    )

    samples_filename = f"samples_{point[0]}_{point[1]}.bin"
    samples_path = context.artifacts((x, y)).samples_path
    shutil.move(samples_filename, samples_path)

    samples = variance.read_bin(samples_path)
    for i in range(power + 1):
        errors = variance.errors(samples, 2 ** i, context.gt_pixel(point))
        print(2 ** i, errors)

@cli.command()
@click.argument("scene_name")
@click.argument("pdf_count", type=int)
@click.argument("checkpoint_name", type=str)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
def pipeline(scene_name, pdf_count, checkpoint_name, output_name, comment):
    dataset_name = scene_name
    context = Context(
        scene_name=scene_name,
        checkpoint_name=checkpoint_name,
        output_name=output_name,
        comment=comment
    )

    results_path = Path("./results").absolute()
    results_path.mkdir(exist_ok=True)

    dataset_path = context.dataset_path(dataset_name)

    phases = [ ("train", pdf_count), ("test", pdf_count)  ]
    for phase, count in phases:
        raw_path = dataset_path / phase / "raw"
        renders_path = dataset_path / phase / "renders"
        raw_path.mkdir(parents=True, exist_ok=True)

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-training.xml"),
            "whatever.exr",
            [],
            {
                "width": dimensions[scene_name][0],
                "height": dimensions[scene_name][1],
                "pdfCount": count,
            }
        )

        for artifact_path in results_path.glob("*"):
            shutil.move(str(artifact_path), str(raw_path))

        process_raw_to_renders.run(
            500,
            raw_path,
            renders_path
        )

        runner.run_nsf_command(
            context.server_path,
            "build_render_dataset.py",
            [ phase, dataset_name ]
        )

    runner.run_nsf_command(
        context.server_path,
        "experiments/plane.py",
        [
            "--dataset_name", context.checkpoint_name,
            "--dataset_path", dataset_path,
            "--num_training_steps", "100000",
        ]
    )

    shutil.move(
        context.server_path / "roots/tmp/decomposition-flows/checkpoints" / f"{context.checkpoint_name}.t",
        context.checkpoint_path
    )

    _render(context, False, False, False, 80, 1)

if __name__ == "__main__":
    cli()
