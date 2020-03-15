import itertools
import json
import tempfile
from pathlib import Path

import exr_to_srgb
import runner

def write_temp_scene(scene_file, distribution, alpha):
    default_json = {
        "sensor": {
            "lookAt": {
                "origin": [ "3.04068", "3.17153", "3.20454" ],
                "target": [ "0.118789", "0.473398", "0.161081" ],
                "up": [ "0", "1", "0" ]
            },
            "fov": "20.114293"
        },
        "materials": [
            {
                "name": "ball",
                "type": "microfacet",
                "distribution": {
                    "type": "beckmann",
                    "alpha": "0.2"
                }
            }
        ],
        "models": [
            {
                "type": "obj",
                "filename": "assets/material-ball/Mesh000.obj",
                "bsdf": {
                    "type": "lambertian",
                    "diffuseReflectance": [ "0.2", "0.2", "0.2" ]
                },
                "transform": {
                    "translate": [
                        "0.110507",
                        "0.494301",
                        "0.126194"
                    ],
                    "scale": [ "0.482906", "0.482906", "0.482906" ]
                }
            },
            {
                "type": "obj",
                "filename": "assets/material-ball/Mesh001.obj",
                "bsdf": {
                    "type": "reference",
                    "name": "ball"
                },
                "transform": {
                    "translate": [
                        "0.0571719",
                        "0.213656",
                        "0.0682078"
                    ],
                    "scale": [ "0.482906", "0.482906", "0.482906" ]
                }
            },
            {
                "type": "obj",
                "filename": "assets/material-ball/Mesh002.obj",
                "bsdf": {
                    "type": "reference",
                    "name": "ball"
                },
                "transform": {
                    "translate": [
                        "0.156382",
                        "0.777229",
                        "0.161698"
                    ],
                    "scale": [ "0.482906", "0.482906", "0.482906" ]
                }
            },
            {
                "type": "quad",
                "transform": {
                    "translate": [
                        "-0.708772",
                        "0",
                        "-0.732108"
                    ],
                    "scale": [ "2.71809", "2.71809", "2.71809" ],
                    "rotate": [
                        "-90",
                        "46.1511",
                        "180"
                    ]
                },
                "bsdf": {
                    "type": "lambertian",
                    "albedo": {
                        "type": "checkerboard",
                        "onColor": [ "0.725", "0.71", "0.68" ],
                        "offColor": [ "0.325", "0.31", "0.25" ],
                        "resolution": { "u": "20", "v": "20" }
                    }
                }
            }
        ],

        "environmentLight": {
            "filename": "assets/material-ball/envmap.exr",
            "transform": {
                "rotate": [ "0", "247.2613999", "0" ]
            },
            "scale": "1"
        }
    }

    default_json["materials"][0]["distribution"]["type"] = distribution
    default_json["materials"][0]["distribution"]["alpha"] = alpha

    scene_file.write(json.dumps(default_json).encode())
    scene_file.flush()

def get_grid_path():
    return Path("grid--ggx")

def get_directory_path(distribution, alpha):
    return get_grid_path() / f"{distribution}-{alpha}"

def create_job_json(scene_path, distribution, alpha):
    job_json = {
        "force": True,
        "spp": 2048,

        "output_directory": str(get_directory_path(distribution, alpha)),
        "output_name": "--",
        "integrator": "PathTracer",
        "scene": str(scene_path),
        "showUI": False,

        "startBounce": 0,
        "lastBounce": 1,

        "width": 640,
        "height": 360
    }

    return job_json

if __name__ == "__main__":
    distributions = [ "ggx", "beckmann" ]
    alphas = [ "0.01", "0.05", "0.1", "0.2", "0.3" ]

    for distribution, alpha in itertools.product(distributions, alphas):
        temp_scene_file = tempfile.NamedTemporaryFile()

        write_temp_scene(temp_scene_file, distribution, alpha)
        job_json = create_job_json(Path(temp_scene_file.name), distribution, alpha)

        runner.run_renderer(job_json)

        output_path = Path("..") / get_directory_path(distribution, alpha)
        exr_to_srgb.convert(
            output_path / "auto.exr",
            Path("..") / get_grid_path() / f"{output_path.name}-ours.png"
        )
