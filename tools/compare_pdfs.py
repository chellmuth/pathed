import math

import numpy as np
import pyexr

def normalize(exr):
    return exr / sum(exr)

def reshape(exr):
    return exr[:, :, 0].flatten()

def compare(exr1, exr2):
    assert exr1.shape == exr2.shape

    exr1 = normalize(reshape(exr1.astype(np.float64)))
    exr2 = normalize(reshape(exr2.astype(np.float64)))

    divergence = 0
    for i, (p, q) in enumerate(zip(exr1, exr2)):
        if p != 0. and q != 0. :
            term = p * math.log(p / q)
            if math.isnan(term):
                breakpoint()
            divergence += term

    return divergence

def run(filename1, filename2):
    return compare(
        pyexr.read(filename1),
        pyexr.read(filename2)
    )

if __name__ == "__main__":
    run("/tmp/cbox-bw--debug/render_384_240.exr", "/tmp/cbox-bw--debug/batch_384_240.exr")
    run("/tmp/cbox-bw--debug/render_20_134.exr", "/tmp/cbox-bw--debug/batch_20_134.exr")

    run("/tmp/cbox-bw--debug/render_20_134.exr", "/tmp/cbox-bw--debug/batch_384_240.exr")
    run("/tmp/cbox-bw--debug/render_384_240.exr", "/tmp/cbox-bw--debug/batch_20_134.exr")
