import subprocess
import time
from multiprocessing import Process
from pathlib import Path

import runner
from dataset_info import DatasetInfo

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


def run_path_tracer(job_json, output_directory):
    if check_freeze(output_directory):
        dummy = Process(target=runner.skip, args=(output_directory,))
        dummy.start()

        return dummy

    path_process = Process(target=runner.run_renderer, args=(job_json,))
    path_process.start()

    return path_process

def run_our_render(*, spp, checkpoint_path, output_name, output_directory, scene, server_directory, port_offset):
    if check_freeze(output_directory):
        dummy1 = Process(target=runner.skip, args=(output_directory,))
        dummy2 = Process(target=runner.skip, args=(output_directory,))
        dummy1.start()
        dummy2.start()

        return dummy1, dummy2

    server_process = Process(target=runner.run_server, args=(server_directory, port_offset, checkpoint_path))
    server_process.start()

    time.sleep(10) # make sure server starts up

    job_json = custom_json(
        spp=spp,
        port_offset=port_offset,
        output_directory=output_directory,
        integrator="DataParallelIntegrator",
        scene=scene,
        output_name=output_name
    )
    render_process = Process(target=runner.run_renderer, args=(job_json,))
    render_process.start()

    return server_process, render_process

def freeze(directory):
    freeze_file = Path(directory) / ".freeze"
    freeze_file.touch(exist_ok=True)

def check_freeze(directory):
    freeze_file = Path(directory) / ".freeze"
    return freeze_file.exists()

def go(experiment_name, dataset_info, server_directory, checkpoint_path, spp, iteration):
    scene = dataset_info.scene("test", iteration)
    scene_json = str(scene) + ".json"

    ours_out = str(dataset_info.experiment_path(experiment_name, iteration))
    # ours_one_out = str(experiment_path / f"test-{iteration}-one")
    path_out = str(dataset_info.comparison_path(f"path-{iteration}"))
    gt_out = str(dataset_info.comparison_path(f"gt-{iteration}"))

    p1, p2 = run_our_render(
        spp=spp,
        scene=scene_json,
        checkpoint_path=checkpoint_path,
        output_name="Ours",
        output_directory=ours_out,
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
        spp=spp,
        output_directory=path_out,
        integrator="PathTracer",
        scene=scene_json,
        output_name="Path",
    )
    path_process = run_path_tracer(path_job_json, path_out)

    gt_job_json = custom_json(
        spp=2 ** 12,
        output_directory=gt_out,
        integrator="PathTracer",
        scene=scene_json,
        output_name="GT",
    )
    gt_process = run_path_tracer(gt_job_json, gt_out)

    p1.join()
    p2.join()
    freeze(ours_out)

    # p3.join()
    # p4.join()

    path_process.join()
    freeze(path_out)

    gt_process.join()
    freeze(gt_out)

    print("We're done!")
    print("python error_reports.py {} --gt {}/auto.exr --includes {} --includes {}".format(
        scene, gt_out, ours_out, path_out
    ))

if __name__ == "__main__":
    spp = 32

    dataset_info = DatasetInfo("/home/cjh/workpad/cornell-dataset")
    scene_directory = dataset_info.scene_path("test")
    nsf_path = Path("/home/cjh/workpad/src/nsf/")
    server_directory = str(nsf_path)

    experiment_name = "20191019-24-bins"
    checkpoint_path = nsf_path / f"roots/tmp/decomposition-flows/checkpoints/{experiment_name}.t"

    iterations = [
        f"{i:04d}"
        for i in range(10)
    ]
    print(iterations)

    for iteration in iterations:
        go(experiment_name, dataset_info, server_directory, checkpoint_path, spp, iteration)
