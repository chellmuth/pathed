from pathlib import Path

from mitsuba import run_mitsuba

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
    mitsuba_path = Path("/home/cjh/workpad/src/mitsuba")
    fisheye_scene_path = mitsuba_path / "cornell-box/scene-training.xml"
    neural_scene_path = mitsuba_path / "cornell-box/scene-neural.xml"

    output_root = Path("/tmp/test-fixed")
    output_root.mkdir(exist_ok=True, parents=True)

    run_mitsuba(
        mitsuba_path,
        fisheye_scene_path,
        output_root / "training.exr",
        [ "-p1" ],
        {
            "x": 100,
            "y": 200,
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
            "x": 100,
            "y": 200,
            "width": 400,
            "height": 400,
        }
    )
