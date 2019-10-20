from pathlib import Path

class DatasetInfo:
    def __init__(self, root):
        self.root = Path(root)

        self.train = self.root / "train"
        self.test = self.root / "test"

        self.train_scenes = self.train / "scenes"
        self.test_scenes = self.test / "scenes"

        self.train_renders = self.train / "renders"
        self.test_renders = self.train / "renders"

        self.train_data = self.train / "data"

    def check_and_create(self):
        directories = [
            self.root,
            self.train,
            self.test,
            self.train_scenes,
            self.test_scenes,
            self.train_renders,
            self.test_renders,
            self.train_data,
        ]

        for directory in directories:
            directory.mkdir(exist_ok=True)

    def scene_path(self, mode):
        if mode == "train":
            return self.train_scenes
        if mode == "test":
            return self.test_scenes

        raise ValueError("Unsupported mode")

    def scene(self, mode, iteration):
        scene_root = self.scene_path(mode)
        return scene_root / f"cornell-{iteration}"

    def render_path(self, mode):
        if mode == "train":
            return self.train_renders
        if mode == "test":
            return self.test_renders

        raise ValueError("Unsupported mode")

    def experiment_path(self, experiment):
        path = self.root / "experiments" / experiment
        path.mkdir(parents=True, exist_ok=True)

        return path
