from multiprocessing import Pool
from pathlib import Path

import click
import numpy as np
import pyexr
from PIL import Image

def save_png(image, output_path):
    uint8_image = (image * 255).astype(np.uint8)

    pil_image = Image.fromarray(uint8_image)
    pil_image.save(str(output_path))

def to_srgb(linear_image):
    a = 0.055
    gamma = 2.4

    return np.where(
        linear_image <= 0.0031308,
        12.92 * linear_image,
        (1 + a) * linear_image ** (1 / gamma) - a
    )

def clip(image):
    return np.clip(image, 0.0, 1.0)

def convert(image, png_path):

    srgb = clip(to_srgb(image))
    save_png(srgb, png_path)

def grow(image, factor):
    return image.repeat(factor, axis=0).repeat(factor, axis=1)

def add_points(image, points):
    colored = np.copy(image)

    assert len(points) <= 3

    colors = [ (1, 0, 0), (0, 1, 0), (0, 0, 1) ]

    for point, color in zip(points, colors):
        colored[point[1], point[0]] = color

    return colored

@click.command()
@click.argument("exr_path", type=click.Path(exists=True))
@click.argument("png_path", type=click.Path(exists=False, file_okay=True))
@click.option("--upsample", type=click.Choice(["1", "2", "4", "8"]), default="1")
@click.option("--point", type=int, nargs=2, multiple=True)
def init(exr_path, png_path, upsample, point):
    upsample = int(upsample)
    points = point

    image = pyexr.read(str(exr_path))

    if point:
        image = add_points(image, points)

    if upsample > 1:
        image = grow(image, upsample)

    convert(image, png_path)

if __name__ == "__main__":
    init()
