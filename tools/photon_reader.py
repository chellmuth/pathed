import struct

from dataclasses import dataclass
from pathlib import Path

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


def _chunker(raw_photons_data):
    photon_count = len(raw_photons_data) // PhotonSize

    return [
        raw_photons_data[i:i + PhotonSize]
        for i in range(photon_count)
    ]

def read_photon_file(photon_path):
    with open(photon_path, "rb") as f:
        data = f.read(FloatSize)
        count, = struct.unpack("i", data)

        float_count = count * PhotonSize
        data = f.read(float_count * FloatSize)
        photons = struct.unpack(f"{float_count}f", data)
        
        print(count)
        for raw_photon in _chunker(photons):
            photon_data = PhotonData(*raw_photon)
            print(photon_data)


if __name__ == "__main__":
    read_photon_file(Path("/home/cjh/workpad/src/mitsuba/photons.bin"))
