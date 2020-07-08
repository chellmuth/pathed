import datetime
import glob
import os
import shutil
import time
from pathlib import Path

import click
import colorama
import numpy as np
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
from context import BaseContext, Context, CheckpointType
from mitsuba import run_mitsuba
from parameters import *

default_scenes = [
    "cbox-ppg",

    "bathroom-diffuse",
    "bathroom2-diffuse",
    "bedroom-diffuse",
    "classroom-diffuse",
    "kitchen-diffuse",
    "living-room-2-diffuse",
    "living-room-3-diffuse",
    "living-room-diffuse",
    "staircase2-diffuse",
]

default_viz_points = {
    "staircase": [
        (12/45, 39/80),
        (11/45, 5/80),
        (23/45, 64/80),
    ],
    "staircase2-diffuse": [
        (0.3, .78),
        (0.6, 0.1),
        (0.9, 0.4),
    ],
    "kitchen-diffuse": [
        (0.125, 0.3333),
        (0.4, 0.8222),
        (0.075, 0.9111),
    ],
    "cbox-ppg": [
        (0.1667, 0.25), # (10, 15),
        (0.5333, 0.6167), # (32, 37),
        (0.1, 0.8334), # (6, 50),
        (0.4333, 0.4333), # (26, 26),

        # (0.6075, 0.53),
        # (0.6175, 0.53),
        # (0.6275, 0.53),
        # (0.6375, 0.53),
        # (0.6475, 0.53),
        # (0.6575, 0.53),

        # (0.70, 0.49),
        # (0.70, 0.51),
        # (0.70, 0.53),
        # (0.70, 0.55),
        # (0.70, 0.57),
        # (0.70, 0.59),
        # (0.70, 0.61),
    ],
    "dining-room": [
        (10/80, 15/40),
        (32/80, 37/40),
        (6/80, 41/40),
    ],
    "living-room-diffuse": [
        (143/800, 291/400),
        (705/800, 305/400),
        (359/800, 312/400),
        (455/800, 27/400),
    ],
    "living-room-2-diffuse": [
        (10/80, 15/40),
        (32/80, 37/40),
        (50/80, 20/40),
    ],
    "living-room-3-diffuse": [
        (10/80, 15/40),
        (32/80, 37/40),
        (50/80, 20/40),
    ],
    "bedroom-diffuse": [
        (10/80, 15/40),
        (32/80, 37/40),
        (50/80, 20/40),
    ],
    "bathroom-diffuse": [
        (10/50, 15/50),
        (32/50, 37/50),
        (45/50, 20/50),
    ],
    "bathroom2-diffuse": [
        (10/80, 15/40),
        (32/80, 37/40),
        (40/80, 20/40),
    ],
    "classroom-diffuse": [
        (10/80, 15/40),
        (32/80, 37/40),
        (50/80, 20/40),
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

def log(line):
    print(colorama.Fore.YELLOW + colorama.Style.BRIGHT + line)

@click.group()
def cli():
    pass

@cli.command()
@click.option("--scene", "scenes", type=str, multiple=True)
@click.option("--all", is_flag=True)
def normalize(scenes, all):
    if all:
        scenes = default_scenes

    if len(scenes) == 1:
        checkpoint_type = CheckpointType.Overfit(scenes[0])
    else:
        checkpoint_type = CheckpointType.General()

    context = BaseContext(checkpoint_type=checkpoint_type)
    _normalize(context, scenes)

def _normalize(context, scenes):
    dataset_paths = [
        context.dataset_path(scene_name)
        for scene_name in scenes
    ]

    full_data = np.zeros((0, RichPhotonLength))
    for dataset_path in dataset_paths:
        data_path = dataset_path / "train" / "data"
        for rich_path in data_path.glob("rich-*.npy"):
            data = np.load(rich_path)
            full_data = np.concatenate((full_data, data))

    mins = np.amin(full_data, axis=0)
    maxes = np.amax(full_data, axis=0)

    # results = (full_data - mins) / (maxes - mins) * 2 - 1.

    combined = np.stack((mins, maxes))
    np.save(
        context.normalize_path,
        combined
    )

    print("Normalizing scenes:", scenes)
    print(f"Saved to: {context.normalize_path}")

@cli.command()
@click.argument("scene_name", type=str)
@click.option("--overfit", is_flag=True, default=False)
@click.option("--all", is_flag=True)
@click.option("--pixel", type=int, nargs=2)
@click.option("--size", type=int, nargs=2)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
def pdf_compare(scene_name, overfit, all, pixel, size, output_name, comment, reuse):
    if overfit:
        checkpoint_type = CheckpointType.Overfit(scene_name)
    else:
        checkpoint_type = CheckpointType.General()

    context = Context(
        scene_name,
        checkpoint_type,
        output_name=output_name,
        comment=comment,
        reuse_output_directory=reuse
    )

    if not size:
        size = dimensions[scene_name]

    if all:
        pixels = [
            (round(p[0] * size[0]), round(p[1] * size[1]))
            for p in default_viz_points[scene_name]
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
            context.current_checkpoint_path,
            context.normalize_path,
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
            context.current_checkpoint_path,
            context.normalize_path,
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
                str(context.current_checkpoint_path),
                str(context.normalize_path),
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
@click.argument("scene_name", type=str)
@click.option("--overfit", is_flag=True, default=False)
@click.option("--size", type=int, default=10)
@click.option("--spp", type=int, default=1)
@click.option("--skip-neural", is_flag=True)
@click.option("--skip-path", is_flag=True)
@click.option("--include-gt", is_flag=True)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
def render(scene_name, overfit, size, spp, skip_neural, skip_path, include_gt, output_name, comment, reuse):
    if overfit:
        checkpoint_type = CheckpointType.Overfit(scene_name)
    else:
        checkpoint_type = CheckpointType.General()

    context = Context(
        scene_name,
        checkpoint_type,
        output_name=output_name,
        comment=comment,
        reuse_output_directory=reuse
    )
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
            context.current_checkpoint_path,
            context.normalize_path,
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

def _generate_training_samples(context, allotment):
    scene_name = context.scene_name
    dataset_name = scene_name
    dataset_path = context.dataset_path(dataset_name)

    results_path = Path("./results").absolute()
    results_path.mkdir(exist_ok=True)

    phases = [ ("train", allotment, _generate_seed()), ("test", min(allotment, 1), None)  ]
    for phase, allotment, seed in phases:
        log(f"Generating raw {phase} data...")
        raw_path = dataset_path / phase / "raw"
        raw_path.mkdir(parents=True, exist_ok=True)

        args = {
            "width": dimensions[scene_name][0],
            "height": dimensions[scene_name][1],
            "minutesAllotment": allotment,
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
            pdf_destination_name = f"pdf_noseed-id_{i}-depth_0-block_0x0.exr"
            os.replace(
                pdf_path,
                raw_path / pdf_destination_name
            )

            photons_path = Path(f"photons_{pixel[0]}_{pixel[1]}.bin")
            photons_destination_name = f"photons_noseed-id_{i}-depth_0-block_0x0.bin"
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
@click.option("--scene", "scenes", type=str, multiple=True)
@click.option("--minutes", type=int)
@click.option("--all", is_flag=True)
def generate_samples(scenes, minutes, all):
    if all:
        scenes = default_scenes

    for scene_name in scenes:
        context = Context(scene_name=scene_name)
        _generate_training_samples(context, minutes)
        _process_training_data(context)

@cli.command()
@click.option("--scene", "scenes", type=str, multiple=True)
@click.option("--skip", "skip_scene", type=str)
@click.option("--steps", type=int, default=10000)
@click.option("--comment", type=str)
@click.option("--output-name", type=str)
def train(scenes, skip_scene, steps, comment, output_name):
    if skip_scene:
        scenes = default_scenes[:]
        scenes.remove(skip_scene)

    if len(scenes) > 1:
        checkpoint_type = CheckpointType.General()
        context = BaseContext(checkpoint_type=checkpoint_type, comment=comment, output_name=output_name)
    else:
        scene_name = scenes[0]
        checkpoint_type = CheckpointType.Overfit(scene_name)
        context = Context(scene_name, checkpoint_type, comment=comment, output_name=output_name)

    dataset_paths = [
        context.dataset_path(scene_name)
        for scene_name in scenes
    ]

    viz_path = context.dataset_path(skip_scene)
    _train(context, steps, dataset_paths, viz_path)

    print("Training complete!")
    print(str(context.output_root))

def _train(context, steps, dataset_paths=None, viz_path=None):
    if dataset_paths is None:
        dataset_name = context.scene_name
        dataset_paths = [ context.dataset_path(dataset_name) ]

    checkpoint_name = context.next_checkpoint_name
    checkpoint_path = context.full_checkpoint_path(checkpoint_name)

    args = [
        "--dataset_name", checkpoint_name,
        "--dataset_paths", *[str(p) for p in dataset_paths],
        "--normalization_path", str(context.normalize_path),
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
    for viz_filename in glob.glob(str(server_out_path) + f"/{checkpoint_name}-*.png"):
        print(viz_filename)
        shutil.move(viz_filename, context.output_root)

    shutil.move(
        context.server_path / "roots/tmp/decomposition-flows/checkpoints" / f"{checkpoint_name}.t",
        checkpoint_path
    )

    shutil.move(
        server_out_path / f"test-losses-{checkpoint_name}.png",
        context.output_root / "graph-convergence.png"
    )

@cli.command()
@click.argument("scene_name")
@click.argument("minutes", type=int)
@click.option("--output-name", type=str)
@click.option("--comment", type=str)
@click.option("--reuse", is_flag=True)
@click.option("--steps", type=int, default=10000)
@click.option("--skip-sample-generation", is_flag=True)
@click.option("--skip-training", is_flag=True)
@click.option("--skip-render", is_flag=True)
def pipeline(scene_name, minutes, output_name, comment, reuse, steps, skip_sample_generation, skip_training, skip_render):
    log("Running pipeline!")

    context = Context(
        scene_name=scene_name,
        checkpoint_type=CheckpointType.Overfit(scene_name),
        output_name=output_name,
        comment=comment,
        reuse_output_directory=reuse
    )

    if skip_sample_generation:
        log("Skipping sample generation")
    else:
        _generate_training_samples(context, minutes)
        _process_training_data(context)
        _normalize(context, [scene_name])

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
