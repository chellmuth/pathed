import subprocess
from itertools import chain

def run_mitsuba(mitsuba_path, scene_path, output_path, options, args):
    args_list = chain.from_iterable(("-D", f"{key}={item}") for key, item in args.items() )

    program = [
        "mitsuba", str(scene_path), "-o", str(output_path), *options, *args_list
    ]

    print(" ".join(program))

    try:
        subprocess.check_output(program)
    except subprocess.CalledProcessError as e:
        print("ERROR!")
        print(" ".join(program))
        raise e
        
