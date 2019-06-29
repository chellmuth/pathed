from pathlib import Path

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

@click.command()
@click.argument("data_sources", nargs=-1)
def combine(data_sources):
    print(data_sources)
    datasets = build_datasets(data_sources)

    for i, spp_set in enumerate(zip(*datasets)):
        combined = spp_set[0]
        for bounce in spp_set[1:]:
            combined += bounce

        exr_name = f"out-{2**i:04d}.exr"
        print(f"Writing: {exr_name}")
        pyexr.write(exr_name, combined)

if __name__ == "__main__":
    combine()
