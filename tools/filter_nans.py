import click
import numpy as np
import pyexr

def neighborhood_average(exr, index):
    neighbors = [
        (
            index[0] + y,
            index[1] + x,
            index[2]
        )
        for y in range(-1, 2)
        for x in range(-1, 2)
        if not (y == 0 and x == 0)
    ]

    height, width, _ = exr.shape
    valid_neighbors = [
        neighbor
        for neighbor
        in neighbors
        if 0 <= neighbor[0] < height
        and 0 <= neighbor[1] < width
    ]

    return sum(
        exr[tuple(neighbor)]
        for neighbor
        in valid_neighbors
    ) / len(valid_neighbors)

@click.command()
@click.argument("input_filename")
@click.argument("output_filename")
def run(input_filename, output_filename):
    exr = pyexr.read(input_filename)

    height, width, channels = exr.shape

    nan_indices = np.concatenate([
        np.argwhere(np.isinf(exr)),
        np.argwhere(np.isnan(exr))
    ])

    for index in nan_indices:
        exr[tuple(index)] = neighborhood_average(exr, index)

    pyexr.write(output_filename, exr)

    print(f"Removed {len(nan_indices)} nans")


if __name__ == "__main__":
    run()
