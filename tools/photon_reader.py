import struct
from dataclasses import dataclass
from pathlib import Path
from typing import List

from phi_theta_grid import PhiThetaGrid, PhotonGridAdapter
from vector import Vector

FloatSize = 4
PhotonSize = 9

def as_json(photon_path: Path):
    bundle = read_photon_bundle(photon_path)
    return {
        "QueryPoint": [ bundle.position.x, bundle.position.y, bundle.position.z ],
        "Results": [
            {
                "point": [ photon.position_x, photon.position_y, photon.position_z ],
                "source": [ photon.source_x, photon.source_y, photon.source_z ],
                "throughput": [ photon.power_r, photon.power_g, photon.power_b ],
            }
            for photon in bundle.photons
        ]
    }

@dataclass
class PhotonData:
    position_x: float
    position_y: float
    position_z: float

    source_x: float
    source_y: float
    source_z: float

    power_r: float
    power_g: float
    power_b: float

    def __repr__(self):
        return " - ".join([
            f"position: ({self.position_x:f}, {self.position_y:f}, {self.position_z:f})",
            f"source: ({self.source_x:f}, {self.source_y:f}, {self.source_z:f})",
            f"power: ({self.power_r:f}, {self.power_g:f}, {self.power_b:f})"
        ])

    @property
    def position(self):
        return Vector(self.position_x, self.position_y, self.position_z)

    @property
    def source(self):
        return Vector(self.source_x, self.source_y, self.source_z)

    @property
    def power(self):
        return [ self.power_r, self.power_g, self.power_b ]

@dataclass
class PhotonBundle:
    position: Vector
    normal: Vector
    wi: Vector

    photons: List[PhotonData]

def _chunker(raw_photons_data):
    photon_count = len(raw_photons_data) // PhotonSize

    return [
        raw_photons_data[i:i + PhotonSize]
        for i in range(photon_count)
    ]

def read_photon_bundle(photon_path: Path):
    with open(photon_path, "rb") as f:
        data = f.read(3 * FloatSize)
        position = struct.unpack("3f", data)

        data = f.read(3 * FloatSize)
        normal = struct.unpack("3f", data)

        data = f.read(3 * FloatSize)
        wi = struct.unpack("3f", data)

        print("position:", position)
        print("normal:", normal)
        print("wi:", wi)

        data = f.read(FloatSize)
        count, = struct.unpack("i", data)

        float_count = count * PhotonSize
        data = f.read(float_count * FloatSize)
        raw_photons = struct.unpack(f"{float_count}f", data)

        photons = [
            PhotonData(*raw_photon)
            for raw_photon
            in _chunker(raw_photons)
        ]

        return PhotonBundle(
            Vector(*position),
            Vector(*normal),
            Vector(*wi),
            photons
        )

def read_photon_file(photon_path):
    grid = PhiThetaGrid(10, 10)

    with open(photon_path, "rb") as f:
        data = f.read(3 * FloatSize)
        position = struct.unpack("3f", data)

        data = f.read(3 * FloatSize)
        normal = struct.unpack("3f", data)

        data = f.read(3 * FloatSize)
        wi = struct.unpack("3f", data)

        print("position:", position)
        print("normal:", normal)
        print("wi:", wi)

        adapter = PhotonGridAdapter(
            Vector(*position),
            Vector(*normal),
            Vector(*wi),
        )

        data = f.read(FloatSize)
        count, = struct.unpack("i", data)

        float_count = count * PhotonSize
        data = f.read(float_count * FloatSize)
        photons = struct.unpack(f"{float_count}f", data)

        print("photons:", count)

        for raw_photon in _chunker(photons):
            photon_data = PhotonData(*raw_photon)
            print(photon_data)

            phi, theta, power = adapter.splat_params(photon_data)
            grid.splat(phi, theta, power)


if __name__ == "__main__":
    read_photon_file(Path("/home/cjh/src/mitsuba/photons.bin"))
