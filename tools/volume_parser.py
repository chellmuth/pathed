import json
import struct

import click
import numpy as np

IntBytes = 4
Float32Bytes = 4

@click.group()
def cli():
    pass


@cli.command()
@click.argument("path", type=click.Path(exists=True))
def convert(path):
    volume_json = json.load(open(path))
    with open("out.vol", "wb") as f:
        f.write(b"VOL")
        f.write(struct.pack("b", 3)) # version
        f.write(struct.pack("i", 1)) # encoding
        f.write(struct.pack("i", volume_json["x"]))
        f.write(struct.pack("i", volume_json["y"]))
        f.write(struct.pack("i", volume_json["z"]))
        f.write(struct.pack("i", 1)) # channels
        f.write(struct.pack("6f", *(volume_json["p0"] + volume_json["p1"]))) # bounds
        f.write(struct.pack(
            f"{volume_json['x'] * volume_json['y'] * volume_json['z']}f",
            *[float(d) for d in volume_json["density"].split(" ")]
        ))

@cli.command()
@click.argument("vol_path", type=click.Path(exists=True))
@click.option("--index", type=int)
@click.option("--print-max", is_flag=True)
def parse(vol_path, index, print_max):
    with open(vol_path, "rb") as f:
        header = f.read(3)
        print(header)
        assert header == b"VOL"

        version, = struct.unpack("b", f.read(1))
        print("version:", version)
        assert version == 3

        encoding, = struct.unpack("i", f.read(IntBytes))
        print("encoding:", encoding)
        assert encoding == 1

        cells_x, = struct.unpack("i", f.read(IntBytes))
        print("cells x:", cells_x)

        cells_y, = struct.unpack("i", f.read(IntBytes))
        print("cells y:", cells_y)

        cells_z, = struct.unpack("i", f.read(IntBytes))
        print("cells z:", cells_z)

        channels, = struct.unpack("i", f.read(IntBytes))
        print("channels:", channels)
        assert channels == 1

        min_x, min_y, min_z, max_x, max_y, max_z = struct.unpack("6f", f.read(Float32Bytes * 6))
        print("bounds:", min_x, min_y, min_z, max_x, max_y, max_z)

        data_points = cells_x * cells_y * cells_z
        data = struct.unpack(f"{data_points}f", f.read(Float32Bytes * data_points)) # (channels == 1)

        if index:
            print("index:", index, data[index])

        if print_max:
            maximum = max(data)
            print("max:", maximum, "index:", data.index(maximum))

if __name__ == "__main__":
    cli()
