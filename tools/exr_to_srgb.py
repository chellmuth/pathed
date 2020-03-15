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

def convert(exr_path, png_path):
    image = pyexr.read(str(exr_path))

    srgb = clip(to_srgb(image))
    save_png(srgb, png_path)

@click.command()
@click.argument("exr_path", type=click.Path(exists=True))
@click.argument("png_path", type=click.Path(exists=False, file_okay=True))
def init(exr_path, png_path):
    convert(exr_path, png_path)

if __name__ == "__main__":
    init()
