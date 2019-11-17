import re
from collections import namedtuple
from pathlib import Path

import photon_reader

Parts = namedtuple("Parts", ["identifier", "block"])

def build_pdf_path(root_path, parts):
    return root_path / f"pdf_{parts.identifier}-block_{parts.block}.bin"

def build_photon_path(root_path, parts):
    return root_path / f"photons_{parts.identifier}-block_{parts.block}.bin"

def pdf_path_parts(pdf_path):
    match = re.match(r"pdf_(\d+)-block_(\d+x\d+).exr", pdf_path.name)
    return Parts(match.group(1), match.group(2))

def photon_path_parts(photon_path):
    match = re.match(r"photons_(\d+)-block_(\d+x\d+).bin", photon_path.name)
    return Parts(match.group(1), match.group(2))

def valid_parts(raw_path):
    parts_to_index = {}

    pdf_paths = raw_path.glob("pdf_*.exr")
    photon_paths = raw_path.glob("photons_*.bin")

    pdf_parts = set()
    photon_parts = set()

    for pdf_path in pdf_paths:
        parts = pdf_path_parts(pdf_path)
        pdf_parts.add(parts)

    for photon_path in photon_paths:
        parts = photon_path_parts(photon_path)
        photon_parts.add(parts)

    parts = pdf_parts.intersection(photon_parts)
    return sorted(parts)

def run(raw_path, renders_path):
    renders_path.mkdir(exist_ok=True, parents=True)

    dat_root = renders_path / "iteration-0000"
    dat_root.mkdir(exist_ok=True)

    parts_to_index = {}

    for i, parts in enumerate(valid_parts(raw_path)):
        pdf_path = build_pdf_path(raw_path, parts)
        # do stuff

        photon_path = build_photon_path(raw_path, parts)

        grid = photon_reader.build_grid(photon_path, (10, 10))
        grid.export_dat(dat_root / f"photon-bundle_{i:05d}.dat")

if __name__ == "__main__":
    run(
        Path("/home/cjh/workpad/Dropbox/research/datasets/mitsuba/raw"),
        Path("/home/cjh/workpad/Dropbox/research/datasets/mitsuba/train/renders"),
    )
