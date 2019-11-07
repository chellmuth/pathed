import struct
from dataclasses import dataclass
from pathlib import Path

from phi_theta_grid import PhiThetaGrid, PhotonGridAdapter
from vector import Vector

FloatSize = 4
PhotonSize = 9

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
            f"({self.position_x:f}, {self.position_y:f}, {self.position_z:f})",
            f"({self.source_x:f}, {self.source_y:f}, {self.source_z:f})",
            f"({self.power_r:f}, {self.power_g:f}, {self.power_b:f})"
        ])

    @property
    def source(self):
        return Vector(self.source_x, self.source_y, self.source_z)

    @property
    def power(self):
        return [ self.power_r, self.power_g, self.power_b ]


def _chunker(raw_photons_data):
    photon_count = len(raw_photons_data) // PhotonSize

    return [
        raw_photons_data[i:i + PhotonSize]
        for i in range(photon_count)
    ]

def read_photon_file(photon_path):
    grid = PhiThetaGrid(10, 10)

    with open(photon_path, "rb") as f:
        data = f.read(3 * FloatSize)
        position = struct.unpack("3f", data)

        data = f.read(3 * FloatSize)
        normal = struct.unpack("3f", data)

        print(position)
        print(normal)

        adapter = PhotonGridAdapter(Vector(*position))

        data = f.read(FloatSize)
        count, = struct.unpack("i", data)

        float_count = count * PhotonSize
        data = f.read(float_count * FloatSize)
        photons = struct.unpack(f"{float_count}f", data)

        print(count)

        for raw_photon in _chunker(photons):
            photon_data = PhotonData(*raw_photon)
            print(photon_data)

            phi, theta, power = adapter.splat_params(photon_data)
            grid.splat(phi, theta, power)


if __name__ == "__main__":
    read_photon_file(Path("/home/cjh/src/mitsuba/photons.bin"))
