import datetime
import glob
import os
import re
import shutil
import time
from collections import namedtuple
from pathlib import Path

import click
import colorama
import pyexr

colorama.init(autoreset=True)

import compare_pdfs
import error_reports
import exr_to_srgb
import photon_reader
import process_raw_to_renders
import simple_chart
import runner
import variance
import visualize
from mitsuba import run_mitsuba
from parameters import GridShape

default_scene_name = "kitchen-diffuse"
default_output_name = "mis"

default_checkpoints = {
    "kitchen": None,
    "cbox-ppg": "20191205-cbox-ppg-2",
    "cbox-bw": "20191217-cbox-bw-4",
}

# Convert to (0, 1) x (0, 1)
default_viz_points = {
    "staircase": [
        (12, 39),
        (11, 5),
        (23, 64),
    ],
    "staircase2-diffuse": [
        (15, 39),
        (30, 5),
        (45, 20),
    ],
    "kitchen-diffuse": [
        (0.125, 0.3333),
        (0.4, 0.8222),
        (0.075, 0.9111),
    ],
    "cbox-ppg": [
        # (0.1667, 0.25), # (10, 15),
        # (0.5333, 0.6167), # (32, 37),
        # (0.1, 0.8334), # (6, 50),
        # (0.4333, 0.4333), # (26, 26),

        # (0.6075, 0.53),
        # (0.6175, 0.53),
        # (0.6275, 0.53),
        # (0.6375, 0.53),
        # (0.6475, 0.53),
        # (0.6575, 0.53),

        (0.70, 0.49),
        (0.70, 0.51),
        (0.70, 0.53),
        (0.70, 0.55),
        (0.70, 0.57),
        (0.70, 0.59),
        (0.70, 0.61),
    ],
    "dining-room": [
        (10, 15),
        (32, 37),
        (6, 41),
    ],
    "living-room-diffuse": [
        (10, 15),
        (32, 37),
        (50, 20),
    ],
    "living-room-2-diffuse": [
        (10, 15),
        (32, 37),
        (50, 20),
    ],
    "living-room-3-diffuse": [
        (10, 15),
        (32, 37),
        (50, 20),
    ],
    "bedroom-diffuse": [
        (10, 15),
        (32, 37),
        (50, 20),
    ],
    "bathroom-diffuse": [
        (10, 15),
        (32, 37),
        (45, 20),
    ],
    "bathroom2-diffuse": [
        (10, 15),
        (32, 37),
        (40, 20),
    ],
    "classroom-diffuse": [
        (10, 15),
        (32, 37),
        (50, 20),
    ],
}

dimensions = {
    "kitchen": (1280, 720),
    "kitchen-diffuse": (80, 45),
    "cbox-ppg": (60, 60),
    "cbox-bw": (400, 400),
    "green-bounce": (80, 45),
    "staircase": (45, 80),
    "staircase2-diffuse": (50, 50),
    "living-room-diffuse": (80, 40),
    "living-room-2-diffuse": (80, 40),
    "living-room-3-diffuse": (80, 40),
    "classroom-diffuse": (80, 40),
    "bathroom-diffuse": (50, 50),
    "bathroom2-diffuse": (80, 40),
    "bedroom-diffuse": (80, 40),
    "dining-room": (80, 45),
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

def log(line):
    print(colorama.Fore.YELLOW + colorama.Style.BRIGHT + line)

def build_output_root(root_path, output_name, comment, reuse):
    counter = 1
    for filename in glob.glob(str(root_path / output_name) + "-[0-9]*"):
        match = re.search(output_name + r"-(\d+)", filename)
        if match:
            identifier = int(match.group(1))

            if reuse or not [f for f in Path(filename).rglob("*") if f.is_file()]:
                counter = max(counter, identifier)
            else:
                counter = max(counter, identifier + 1)

    dir_name = output_name + f"-{counter}"
    if comment:
        dir_name += f"--{comment}"

    return Path(root_path / dir_name)

def get_default_checkpoint_stem(scene_name, root_path, verbose=False):
    # TODO: handle custom checkpoint names
    checkpoint_files = glob.glob(str(root_path / scene_name) + "-[0-9]*-*")
    if checkpoint_files:
        latest_checkpoint_path = Path(sorted(checkpoint_files)[-1])
        if verbose:
            print(f"Using latest checkpoint: {latest_checkpoint_path}")

        return latest_checkpoint_path.stem

    if verbose:
        print(f"Using default checkpoint: {default_checkpoints[scene_name]}")

    return default_checkpoints[self.scene_name]

def build_next_checkpoint_stem(scene_name, comment, root_path, verbose=False):
    today = datetime.datetime.now().strftime("%Y%m%d")

    prefix = "-".join([token for token in [scene_name, comment, today] if token])

    counter = 1

    checkpoints_pattern = str(root_path / prefix) + "-*"
    checkpoint_filenames = glob.glob(checkpoints_pattern)

    if checkpoint_filenames:
        latest_filename = sorted(checkpoint_filenames)[-1]

        if verbose:
            print(f"Most recent checkpoint: {latest_filename}")

        match = re.search(f"{prefix}-" + r"(\d+)", latest_filename)
        identifier = int(match.group(1))

        counter = max(counter, identifier + 1)

    next_stem = f"{prefix}-{counter:02}"

    if verbose:
        print(f"Next checkpoint: {next_stem}")

    return next_stem

class Context:
    def __init__(self, scene_name=None, next_checkpoint_name=None, output_name=None, comment=None, reuse_output_directory=False):
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
        if next_checkpoint_name is None:
            self.checkpoint_name = get_default_checkpoint_stem(
                self.scene_name,
                self.checkpoint_root,
                verbose=True
            )
        else:
            self.checkpoint_name = build_next_checkpoint_stem(
                self.scene_name,
                next_checkpoint_name,
                self.checkpoint_root,
                verbose=True
            )

        self.checkpoint_path = self.checkpoint_root / f"{self.checkpoint_name}.t"

    @property
    def convergence_plot_path(self):
        return self.output_root / "convergence_plot.png"

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
        exr = pyexr.read(str(self.scenes_path / "gt.exr"))
        return exr[point[1]][point[0]][channel]

    def gt_path(self, width, height):
        return self.scenes_path / f"gt_size{width}x{height}.exr"

@click.group()
def cli():
    pass

@cli.command()
@click.option("--all", is_flag=True)
@click.option("--pixel", type=int, nargs=2)
@click.option("--size", type=int, nargs=2)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
def pdf_compare(all, pixel, size, output_name, comment, reuse):
    context = Context(output_name=output_name, comment=comment, reuse_output_directory=reuse)

    if not size:
        size = dimensions[default_scene_name]

    if all:
        pixels = [
            (round(p[0] * size[0]), round(p[1] * size[1]))
            for p in default_viz_points[default_scene_name]
        ]
    elif pixel:
        pixels = [pixel]
    else:
        print("Nothing to compare!")
        exit(1)


    print(f"Processing pixels: {pixels}")

    for i, pixel in enumerate(pixels):
        color_name = exr_to_srgb.ColorNames[i]

        server_viz_path = context.artifacts(pixel).server_viz_path
        server_process = runner.launch_server(
            context.server_path,
            0,
            context.checkpoint_path,
            server_viz_path
        )

        time.sleep(10) # make sure server starts up

        log("Rendering fisheye...")
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-training.xml"),
            context.output_root / "training.exr",
            [ "-p1" ],
            # [], # "-p1" gives reproducible results
            {
                "x": pixel[0],
                "y": pixel[1],
                "width": size[0],
                "height": size[1],
            },
            verbose=True
        )

        log("Rendering neural 1...")
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-neural.xml"),
            context.output_root / "neural.exr",
            [ "-p1" ],
            {
                "x": pixel[0],
                "y": pixel[1],
                "width": size[0],
                "height": size[1],
                "integrator": "neural",
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

        log("Rendering neural 2...")
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-neural.xml"),
            context.output_root / "neural.exr",
            [ "-p1" ],
            {
                "x": pixel[0],
                "y": pixel[1],
                "spp": 100,
                "width": size[0],
                "height": size[1],
                "integrator": "neural",
            },
            verbose=True
        )

        pdf_out_path = context.output_root / f"pdf_{pixel[0]}_{pixel[1]}.exr"
        photons_grid_out_path = context.output_root / f"photon-grid_{pixel[0]}_{pixel[1]}.dat"
        photons_rich_out_path = context.output_root / f"photon-rich_{pixel[0]}_{pixel[1]}.dat"
        process_raw_to_renders.execute(
            Path(f"render_{pixel[0]}_{pixel[1]}.exr"),
            Path(f"photons_{pixel[0]}_{pixel[1]}.bin"),
            pdf_out_path,
            photons_grid_out_path,
            photons_rich_out_path,
        )

        # run viz
        viz_out_path = context.output_root / f"viz_{i}_{color_name}_{pixel[0]}_{pixel[1]}.png"

        runner.run_nsf_command(
            context.server_path,
            "evaluate.py",
            [
                str(pdf_out_path),
                str(photons_grid_out_path),
                str(photons_rich_out_path),
                str(viz_out_path),
                str(context.checkpoint_path)
            ]
        )

        batch_path = Path(f"batch_{pixel[0]}_{pixel[1]}.exr")
        neural_out_path = context.output_root / f"neural_{pixel[0]}_{pixel[1]}.png"
        visualize.convert_to_density_mesh(
            batch_path,
            neural_out_path
        )

        grid_path = Path(f"grid_{pixel[0]}_{pixel[1]}.bin")
        grid = photon_reader.read_raw_grid(grid_path, GridShape)

        from matplotlib import cm, pyplot as plt

        figure, axes = plt.subplots(1, 1)
        visualize.photon_bundle(axes, grid)

        plt.tight_layout()
        plt.savefig(context.output_root / f"grid_{pixel[0]}_{pixel[1]}.png", bbox_inches='tight')
        plt.close()

        # Cleanup
        render_filename = f"render_{pixel[0]}_{pixel[1]}.exr"
        batch_filename = f"batch_{pixel[0]}_{pixel[1]}.exr"
        samples_filename = f"samples_{pixel[0]}_{pixel[1]}.bin"

        shutil.move(render_filename, context.output_root / render_filename)
        shutil.move(batch_filename, context.output_root / batch_filename)

        grid_filename = f"grid_{pixel[0]}_{pixel[1]}.bin"
        photons_filename = f"photons_{pixel[0]}_{pixel[1]}.bin"
        neural_filename = f"neural_{pixel[0]}_{pixel[1]}.bin"
        shutil.move(grid_filename, context.output_root / grid_filename)
        shutil.move(photons_filename, context.output_root / photons_filename)
        shutil.move(neural_filename, context.output_root / neural_filename)
        shutil.move(samples_filename, context.output_root / samples_filename)
        shutil.move(server_viz_path, context.output_root / server_viz_path.name)

    gt_path = context.gt_path(size[0], size[1])
    if gt_path.exists():
        exr = pyexr.read(str(gt_path))
        annotated_gt = exr_to_srgb.add_points(exr, pixels)

        pyexr.write(str(context.output_root / "map.exr"), annotated_gt)

    print("Finished pdf analysis!")
    print(f"tev {context.output_root}/viz*.png {context.output_root}/map.exr")
    print(context.output_root)


# Quick 1spp comparison between neural and path
@cli.command()
@click.option("--size", type=int, default=10)
@click.option("--spp", type=int, default=1)
@click.option("--skip-neural", is_flag=True)
@click.option("--skip-path", is_flag=True)
@click.option("--include-gt", is_flag=True)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
def render(size, spp, skip_neural, skip_path, include_gt, output_name, comment, reuse):
    context = Context(output_name=output_name, comment=comment, reuse_output_directory=reuse)
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

    gt_path = context.gt_path(width, height)
    if include_gt and not gt_path.exists():
        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-path.xml"),
            gt_path,
            [],
            {
                "width": width,
                "height": height,
                "spp": 2**12,
            }
        )

    if gt_path.exists():
        error_reports.run_with_data_source_objects(
            gt_path,
            [
                error_reports.SingleSPPDataSource("Path", context.output_root / "path.exr"),
                error_reports.SingleSPPDataSource("Ours", context.output_root / "render.exr")
            ],
            out_path=context.convergence_plot_path
        )

        shutil.copyfile(gt_path, context.output_root / "gt.exr")

    shutil.move("g_position1.exr", context.output_root / "g_position1.exr")
    shutil.move("g_normal1.exr", context.output_root / "g_normal1.exr")
    shutil.move("g_position2.exr", context.output_root / "g_position2.exr")
    shutil.move("g_normal2.exr", context.output_root / "g_normal2.exr")
    shutil.move("g_neural_wo.exr", context.output_root / "g_neural_wo.exr")
    shutil.move("g_lit1.exr", context.output_root / "g_lit1.exr")
    shutil.move("g_lit2.exr", context.output_root / "g_lit2.exr")
    shutil.move("g_phi_theta.exr", context.output_root / "g_phi_theta.exr")
    shutil.move("g_local_wi.exr", context.output_root / "g_local_wi.exr")
    shutil.move("g_world_wi.exr", context.output_root / "g_world_wi.exr")
    shutil.move("g_intersection_wi.exr", context.output_root / "g_intersection_wi.exr")

def _generate_seed():
    timestamp_format = "%Y%m%d%H%M%S"
    return datetime.datetime.now().strftime(timestamp_format)

def _generate_training_samples(context, pdf_count):
    scene_name = context.scene_name
    dataset_name = scene_name
    dataset_path = context.dataset_path(dataset_name)

    results_path = Path("./results").absolute()
    results_path.mkdir(exist_ok=True)

    phases = [ ("train", pdf_count, _generate_seed()), ("test", min(pdf_count, 5), None)  ]
    for phase, count, seed in phases:
        log(f"Generating raw {phase} data...")
        raw_path = dataset_path / phase / "raw"
        raw_path.mkdir(parents=True, exist_ok=True)

        args = {
            "width": dimensions[scene_name][0],
            "height": dimensions[scene_name][1],
            "pdfCount": count,
        }
        if seed:
            args["seed"] = seed

        run_mitsuba(
            context.mitsuba_path,
            context.scene_path("scene-training.xml"),
            "whatever.exr",
            [],
            args
        )

        for artifact_path in results_path.glob("*"):
            os.replace(str(artifact_path), str(raw_path / artifact_path.name))

    for phase in [ "viz" ]:
        log(f"Generating raw {phase} data...")
        raw_path = dataset_path / phase / "raw"
        raw_path.mkdir(parents=True, exist_ok=True)

        size = dimensions[scene_name]
        for i, point in enumerate(default_viz_points[scene_name]):
            pixel = (round(size[0] * point[0]), round(size[1] * point[1]))

            run_mitsuba(
                context.mitsuba_path,
                context.scene_path("scene-training.xml"),
                "whatever.exr",
                [ "-p1" ], # deterministic
                {
                    "x": pixel[0],
                    "y": pixel[1],
                    "width": size[0],
                    "height": size[1],
                }
            )

            pdf_path = Path(f"render_{pixel[0]}_{pixel[1]}.exr")
            pdf_destination_name = f"pdf_noseed-id_{i}-block_0x0.exr"
            os.replace(
                pdf_path,
                raw_path / pdf_destination_name
            )

            photons_path = Path(f"photons_{pixel[0]}_{pixel[1]}.bin")
            photons_destination_name = f"photons_noseed-id_{i}-block_0x0.bin"
            os.replace(
                photons_path,
                raw_path / photons_destination_name
            )

def _process_training_data(context):
    dataset_name = context.scene_name
    dataset_path = context.dataset_path(dataset_name)

    for phase in [ "train", "test", "viz" ]:
        log(f"Processing raw {phase} data...")
        raw_path = dataset_path / phase / "raw"
        renders_path = dataset_path / phase / "renders"

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

@cli.command()
@click.option("--steps", type=int, default=10000)
@click.option("--comment", type=str)
@click.option("--output-name", type=str)
def train(steps, comment, output_name):
    context = Context(comment=comment, output_name=output_name)
    dataset_paths = [
        Path("/home/cjh/workpad/Dropbox/research/datasets/bathroom-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/bathroom2-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/bedroom-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/classroom-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/living-room-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/living-room-2-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/living-room-3-diffuse"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/staircase2-diffuse"),
    ]

    viz_path = Path("/home/cjh/workpad/Dropbox/research/datasets/cbox-ppg")
    _train(context, steps, dataset_paths, viz_path)

def _train(context, steps, dataset_paths=None, viz_path=None):
    if dataset_paths is None:
        dataset_name = context.scene_name
        dataset_paths = [ context.dataset_path(dataset_name) ]

    args = [
        "--dataset_name", context.checkpoint_name,
        "--dataset_paths", *dataset_paths,
        "--num_training_steps", str(steps),
    ]

    if viz_path:
        args.extend([
            "--viz_dataset_path", str(viz_path)
        ])

    runner.run_nsf_command(
        context.server_path,
        "experiments/plane.py",
        args
    )

    server_out_path = context.server_path / "roots/tmp/decomposition-flows/out"
    for viz_filename in glob.glob(str(server_out_path) + f"/{context.checkpoint_name}-*.png"):
        print(viz_filename)
        shutil.move(viz_filename, context.output_root)

    shutil.move(
        context.server_path / "roots/tmp/decomposition-flows/checkpoints" / f"{context.checkpoint_name}.t",
        context.checkpoint_path
    )

    shutil.move(
        server_out_path / f"test-losses-{context.checkpoint_name}.png",
        context.output_root / "graph-convergence.png"
    )

@cli.command()
@click.argument("scene_name")
@click.argument("pdf_count", type=int)
@click.option("--checkpoint", type=str)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
@click.option("--steps", type=int, default=10000)
@click.option("--skip-sample-generation", is_flag=True)
@click.option("--skip-training", is_flag=True)
@click.option("--skip-render", is_flag=True)
def pipeline(scene_name, pdf_count, checkpoint, output_name, comment, reuse, steps, skip_sample_generation, skip_training, skip_render):
    log("Running pipeline!")

    context = Context(
        scene_name=scene_name,
        next_checkpoint_name=checkpoint or "",
        output_name=output_name,
        comment=comment,
        reuse_output_directory=reuse
    )

    if skip_sample_generation:
        log("Skipping sample generation")
    else:
        _generate_training_samples(context, pdf_count)
        _process_training_data(context)

    if skip_training:
        log("Skipping training")
    else:
        log("Training...")
        _train(context, steps)

    if skip_render:
        log("Skipping render")
    else:
        log("Rendering...")
        _render(context, False, False, True, dimensions[scene_name][0], 1)

    log("Pipeline complete!")
    print(context.output_root)


if __name__ == "__main__":
    cli()
