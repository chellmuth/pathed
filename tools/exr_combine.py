from pathlib import Path
import json

import click
import pyexr

def build_dataset(data_source):
    exrs = []

    i = 0
    while True:
        data_path = Path(data_source)
        exr_path = data_path / f"auto-{2**i:05d}spp.exr"
        if not exr_path.exists():
            break

        exrs.append(pyexr.read(str(exr_path)))

        i += 1

    return exrs

def build_datasets(data_sources):
    return [
        build_dataset(data_source)
        for data_source
        in data_sources
    ]

def find_scene(data_sources):
    scenes = set()
    for data_source in data_sources:
        report = json.load(open(Path(data_source) / "report.json"))
        scenes.add(report["scene"])

    if len(scenes) > 1:
        raise Exception(f"Different scenes present! {scenes}")

    return scenes.pop()

def write_report_json(out_path, scene):
    content = {
        "scene": scene,
        "output_name": "Combined"
    }

    json.dump(content, open(out_path / "report.json", "w"))

@click.command()
@click.argument("data_sources", nargs=-1)
def combine(data_sources):
    print(data_sources)

    scene = find_scene(data_sources)

    out_path = Path("./combined")
    if not out_path.exists():
        out_path.mkdir()

    datasets = build_datasets(data_sources)

    for i, spp_set in enumerate(zip(*datasets)):
        combined = spp_set[0]
        for bounce in spp_set[1:]:
            combined += bounce

        exr_path = out_path / f"auto-{2**i:04d}.exr"
        print(f"Writing: {exr_path}")
        pyexr.write(str(exr_path), combined)

    write_report_json(out_path, scene)

if __name__ == "__main__":
    combine()
