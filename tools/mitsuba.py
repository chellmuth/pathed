import subprocess
from itertools import chain

def run_mitsuba(mitsuba_path, scene_path, output_path, options, args, verbose=False):
    args_list = chain.from_iterable(("-D", f"{key}={item}") for key, item in args.items() )

    program = [
        "mitsuba", str(scene_path), "-o", str(output_path), *options, *args_list
    ]

    print(" ".join(program))

    try:
        output = subprocess.check_output(program)
        if verbose:
            print(output.decode("utf-8"))
    except subprocess.CalledProcessError as e:
        print("ERROR!")
        print(" ".join(program))
        raise e
        
