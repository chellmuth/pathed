import json
import subprocess
import tempfile
from multiprocessing import Process

def run_renderer(job_json):
    with tempfile.NamedTemporaryFile("w") as f:
        f.write(json.dumps(job_json, indent=2))
        f.flush()

        try:
            output = subprocess.check_call(["./pathed", f.name], cwd="../Release")
        except subprocess.CalledProcessError:
            print("ERROR CALLING RENDERER!!")
            print(job_json)

def run_nsf_command(nsf_root, command, args_list):
    command = ["pipenv", "run", "python", command, *args_list]

    print("Running command:", " ".join([str(c) for c in command]))

    subprocess.check_output(
        command,
        cwd=str(nsf_root)
    )

def run_server(server_path, port_offset, checkpoint_path, viz_path):
    args = [str(port_offset), checkpoint_path]
    if viz_path:
        args.append(str(viz_path))

    run_nsf_command(server_path, "server.py", args)

def launch_server(server_path, port_offset, checkpoint_path, viz_path=None):
    server_process = Process(
        target=run_server,
        args=(server_path, port_offset, checkpoint_path, viz_path)
    )
    server_process.start()
    return server_process

def skip(directory):
    print(f"Skipping {directory}")
