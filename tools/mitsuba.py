import subprocess
from itertools import chain

def run_mitsuba(mitsuba_path, scene_path, output_path, options, args):
    args_list = chain.from_iterable(("-D", f"{key}={item}") for key, item in args.items() )
    subprocess.check_output(
        [
            "mitsuba", str(scene_path), "-o", str(output_path), " ".join(options), *args_list
        ]
    )
