import click
import numpy as np
import pyexr

@click.command()
@click.argument("exr_path", type=click.Path(exists=True))
def init(exr_path):
    exr = pyexr.read(exr_path)

    max_coords = (-1, -1)
    max_value = 0

    height, width, _ = exr.shape

    for y in range(height):
        for x in range(width):
            value = np.sum(exr[y, x, :])
            if value > max_value:
                max_value = value
                max_coords = (x, y)

    print(f"max value: {max_value}")
    print(f"max coords: {max_coords}")


if __name__ == "__main__":
    init()
