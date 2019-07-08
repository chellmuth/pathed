from multiprocessing import Pool
from pathlib import Path

import click
import numpy as np
import pyexr
from PIL import Image

def save_png(image, output_pathname):
    uint8_image = (image * 255).astype(np.uint8)

    pil_image = Image.fromarray(uint8_image)
    pil_image.save(output_pathname)

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

def _convert(hdr_filename, out_directory):
    image = pyexr.read(str(hdr_filename))

    srgb = clip(to_srgb(image))
    output_pathname = str(out_directory / f"{hdr_filename.stem}.png")
    save_png(srgb, output_pathname)

def _convert_wrapper(args):
    print(f"Converting {args}")
    _convert(*args)

@click.command()
@click.argument("root", type=click.Path(exists=True))
@click.argument("out_directory", type=click.Path(exists=False, file_okay=False))
def init(root, out_directory):
    queue = []

    root = Path(root)
    out_directory = Path(out_directory)
    out_directory.mkdir(exist_ok=True)

    for exr_path in root.glob("*.exr"):
        queue.append((exr_path, out_directory))

    pool = Pool(7)
    pool.map(_convert_wrapper, queue)

if __name__ == "__main__":
    init()
