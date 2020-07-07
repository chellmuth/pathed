import datetime
import glob
import os
import re
from pathlib import Path

default_output_name = "output"

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

    return None

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
    def __init__(self, scene_name, next_checkpoint_name=None, output_name=None, comment=None, reuse_output_directory=False):
        self.scene_name = scene_name

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
        self.normalize_path = self.datasets_path / "normalize.npy"

    def next_general_checkpoint_name(self):
        return build_next_checkpoint_stem(
            "generalized",
            None,
            self.checkpoint_root,
            verbose=True
        )

    def build_checkpoint_path(self, checkpoint_name):
        if checkpoint_name:
            return self.checkpoint_root / f"{checkpoint_name}.t"
        else:
            return self.checkpoint_root / f"{self.checkpoint_name}.t"

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
