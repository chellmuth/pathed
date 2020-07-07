import datetime
import glob
import os
import re
from collections import namedtuple
from pathlib import Path

default_output_name = "output"

Artifacts = namedtuple("Artifacts", [ "render_path", "batch_path", "samples_path", "server_viz_path" ])

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

class CheckpointTypeBase:
    pass

class CheckpointType:
    class General(CheckpointTypeBase):
        def current_name(self, checkpoint_root):
            return get_default_checkpoint_stem(
                "generalized",
                checkpoint_root,
                verbose=True
            )

        def next_name(self, checkpoint_root):
            return build_next_checkpoint_stem(
                "generalized",
                None,
                checkpoint_root,
                verbose=True
            )

    class Overfit(CheckpointTypeBase):
        def __init__(self, scene_name):
            self.scene_name = scene_name

        def current_name(self, checkpoint_root):
            return get_default_checkpoint_stem(
                self.scene_name,
                checkpoint_root,
                verbose=True
            )

        def next_name(self, checkpoint_root):
            return build_next_checkpoint_stem(
                self.scene_name,
                None,
                checkpoint_root,
                verbose=True
            )

class BaseContext:
    def __init__(self, checkpoint_type=None, output_name=None, comment=None, reuse_output_directory=False):
        self.mitsuba_path = Path(os.environ["MITSUBA_ROOT"])
        self.server_path = Path(os.environ["NSF_ROOT"])
        self.research_path = Path(os.environ["RESEARCH_ROOT"])
        self.datasets_path = self.research_path / "datasets"
        self.checkpoint_root = self.research_path / "checkpoints"
        self.normalize_path = self.datasets_path / "normalize.npy"

        self.checkpoint_type = checkpoint_type
        self.output_root = build_output_root(
            Path("/tmp"),
            output_name or default_output_name,
            comment,
            reuse_output_directory
        )
        self.output_root.mkdir(exist_ok=True, parents=True)

    def dataset_path(self, dataset_name):
        return self.datasets_path / dataset_name

    @property
    def current_checkpoint_path(self):
        checkpoint_name = self.checkpoint_type.current_name(self.checkpoint_root)
        return self.full_checkpoint_path(checkpoint_name)

    @property
    def next_checkpoint_name(self):
        return self.checkpoint_type.next_name(self.checkpoint_root)

    @property
    def next_checkpoint_path(self):
        return self.full_checkpoint_path(self.next_checkpoint_name)

    def full_checkpoint_path(self, checkpoint_name):
        return self.checkpoint_root / f"{checkpoint_name}.t"

class Context(BaseContext):
    def __init__(self, scene_name, checkpoint_type=None, output_name=None, comment=None, reuse_output_directory=False):
        super().__init__(checkpoint_type)

        self.scene_name = scene_name
        self.scenes_path = self.research_path / "scenes" / self.scene_name

    @property
    def convergence_plot_path(self):
        return self.output_root / "convergence_plot.png"

    def scene_path(self, scene_file):
        return self.scenes_path / scene_file

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
