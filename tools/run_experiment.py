import json
import subprocess
import time
import tempfile
from multiprocessing import Process
from pathlib import Path

def custom_json(*, spp, port_offset=0, output_directory, integrator, scene, output_name):
    return {
        "force": True,

        "spp": spp,
        "port_offset": port_offset,

        "output_directory": output_directory,
        "output_name": output_name,
        "integrator": integrator,
        "scene": scene,
        "showUI": False,

        "startBounce": 2,
        "lastBounce": 2,

        "lightPhiSteps": 20,
        "lightThetaSteps": 20,

        "phiSteps": 10,
        "thetaSteps": 10,
        "debugSearchCount": 100,

        "photonSamples": 100000,
        "photonBounces": 2,

        "width": 400,
        "height": 400
    }


def run_renderer(job_json):
    with tempfile.NamedTemporaryFile("w") as f:
        f.write(json.dumps(job_json, indent=2))
        f.flush()

        try:
            output = subprocess.check_call(["./pathed", f.name], cwd="../Release")
        except subprocess.CalledProcessError:
            print("ERROR CALLING RENDERER!!")
            print(job_json)

def run_server(server_directory, port_offset, checkpoint_path):
    subprocess.check_output(
        ["pipenv", "run", "python", "server.py", str(port_offset), checkpoint_path],
        cwd=server_directory
    )

def run_our_render(*, checkpoint_path, output_name, output_directory, scene, server_directory, port_offset):
    server_process = Process(target=run_server, args=(server_directory, port_offset, checkpoint_path))
    server_process.start()

    time.sleep(10) # make sure server starts up

    job_json = custom_json(
        spp=128,
        port_offset=port_offset,
        output_directory=output_directory,
        integrator="DataParallelIntegrator",
        scene=scene,
        output_name=output_name
    )
    render_process = Process(target=run_renderer, args=(job_json,))
    render_process.start()

    return server_process, render_process

def go(scene_directory, server_directory, iteration):
    experiment_root = Path("/home/cjh/workpad/cornell-dataset/20191018-streaming-dataset")

    scene = f"{scene_directory}/cornell-{iteration}"
    scene_json = scene + ".json"
    ours_one_out = str(experiment_root / f"test-{iteration}-one")
    ours_many_out = str(experiment_root / f"test-{iteration}-many")
    path_out = str(experiment_root / f"test-{iteration}-path")
    gt_out = str(experiment_root / f"test-{iteration}-gt")

    p1, p2 = run_our_render(
        scene=scene_json,
        checkpoint_path="/home/cjh/src/nsf/roots/tmp/decomposition-flows/checkpoints/20191015-streaming-dataset.t",
        output_name="Ours",
        output_directory=ours_many_out,
        server_directory=server_directory,
        port_offset=0,
    )

    # p3, p4 = run_our_render(
    #     scene=scene_json,
    #     checkpoint_path="/home/cjh/workpad/src/nsf/roots/tmp/decomposition-flows/checkpoints/direct-only.t",
    #     output_name="Ours (trained one)",
    #     output_directory=ours_one_out,
    #     server_directory=server_directory,
    #     port_offset=1,
    # )

    path_job_json = custom_json(
        spp=128,
        output_directory=path_out,
        integrator="PathTracer",
        scene=scene_json,
        output_name="Path",
    )
    path_process = Process(target=run_renderer, args=(path_job_json,))
    path_process.start()

    gt_job_json = custom_json(
        spp=2 ** 12,
        output_directory=gt_out,
        integrator="PathTracer",
        scene=scene_json,
        output_name="GT",
    )
    gt_process = Process(target=run_renderer, args=(gt_job_json,))
    gt_process.start()

    p1.join()
    p2.join()
    # p3.join()
    # p4.join()

    path_process.join()
    gt_process.join()

    print("We're done!")
    print("python error_reports.py {} --gt {}/auto.exr --includes {} --includes {} --includes {}".format(
        scene, gt_out, ours_many_out, ours_one_out, path_out
    ))

if __name__ == "__main__":
    scene_directory = "procedural-test"
    server_directory = "/home/cjh/src/nsf/"

    iterations = [
        f"{i:04d}"
        for i in range(10)
    ]
    print(iterations)

    for iteration in iterations:
        go(scene_directory, server_directory, iteration)
