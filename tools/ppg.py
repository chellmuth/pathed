import subprocess
from pathlib import Path
from itertools import chain

def run_mitsuba(mitsuba_path, scene_path, output_path, args):
    args_list = chain.from_iterable(("-D", f"{key}={item}") for key, item in args.items() )
    subprocess.check_output(
        [
            "mitsuba", str(scene_path), "-o", str(output_path), *args_list
        ]
    )

def go(mitsuba_path, scene_path, spp_log_range, args_builder_fn, output_root):
    if not output_root.exists():
        output_root.mkdir(parents=True)

    for spp in [ 2**i for i in range(*spp_log_range) ]:
        output_path = output_root / f"auto-{spp:05d}spp.exr"
        print(f"Generating {output_path}")

        args = args_builder_fn(spp)
        run_mitsuba(mitsuba_path, scene_path, output_path, args)

def build_ppg_args(spp):
    return {
        "budget": spp - 1
    }

def build_path_args(spp):
    return {
        "spp": spp
    }

if __name__ == "__main__":
    ppg_path = Path("/home/cjh/src/practical-path-guiding")

    mitsuba_path = ppg_path / "mitsuba"
    ppg_scene_path = ppg_path / "scenes/cbox/cbox-test.xml"
    path_scene_path = ppg_path / "scenes/cbox/cbox-cjh.xml"

    output_root = Path("/tmp/test-fixed")

    go(mitsuba_path, ppg_scene_path, (5, 10 + 1), build_ppg_args, output_root / "ppg")
    go(mitsuba_path, path_scene_path, (0, 10 + 1), build_path_args, output_root / "path")
