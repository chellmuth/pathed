import re
from collections import namedtuple
from pathlib import Path

import photon_reader

PhotonParts = namedtuple("PhotonParts", ["identifier", "block"])

def build_photon_path(root_path, parts):
    return root_path / f"photons_{parts.identifier}-block_{parts.block}.bin"

def photon_path_parts(photon_path):
    match = re.match(r"photons_(\d+)-block_(\d+x\d+).bin", photon_path.name)
    return PhotonParts(match.group(1), match.group(2))

def run(raw_path, renders_path):
    renders_path.mkdir(exist_ok=True, parents=True)

    for photon_path in raw_path.glob("photons*.bin"):
        grid = photon_reader.build_grid(photon_path, (10, 10))
        grid.export_dat("output.dat")

if __name__ == "__main__":
    run(
        Path("/home/cjh/workpad/Dropbox/research/datasets/mitsuba/raw"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/mitsuba/train/renders"),
    )
