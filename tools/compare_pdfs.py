import math

import numpy as np
import pyexr

def normalize(exr):
    return exr / sum(exr)

def reshape(exr):
    return exr[:, :, 0].flatten()

def kl_divergence(exr1, exr2):
    divergence = 0
    for i, (p, q) in enumerate(zip(exr1, exr2)):
        if p != 0. and q != 0. :
            term = p * math.log(p / q)

            if math.isnan(term):
                breakpoint()

            divergence += term

    return divergence

def chi2_divergence(exr1, exr2):
    divergence = 0
    for i, (p, q) in enumerate(zip(exr1, exr2)):
        if p != 0. and q != 0. :
            term = (p - q) ** 2 / q

            if math.isnan(term):
                breakpoint()

            divergence += term

    return divergence

def absolute_distance(exr1, exr2):
    divergence = 0
    for i, (p, q) in enumerate(zip(exr1, exr2)):
        divergence += abs(p - q)

    return divergence

def squared_distance(exr1, exr2):
    divergence = 0
    for i, (p, q) in enumerate(zip(exr1, exr2)):
        divergence += (p - q) ** 2

    return divergence

def compare(exr1, exr2):
    assert exr1.shape == exr2.shape

    exr1 = normalize(reshape(exr1.astype(np.float64)))
    exr2 = normalize(reshape(exr2.astype(np.float64)))

    return {
        "kl": kl_divergence(exr1, exr2),
        "chi2": chi2_divergence(exr1, exr2),
        "abs": absolute_distance(exr1, exr2),
        "squared": squared_distance(exr1, exr2),
    }

def generate_zero_map(source_path, inferred_path, out_path):
    generated_map = zero_map(
        pyexr.read(str(source_path)),
        pyexr.read(str(inferred_path)),
    )
    pyexr.write(str(out_path), generated_map)

def zero_map(source_exr, inferred_exr):
    h, w, _ = source_exr.shape

    source_exr = reshape(source_exr)
    inferred_exr = reshape(inferred_exr)

    zero_map = np.zeros(source_exr.shape)
    for i, (p, q) in enumerate(zip(source_exr, inferred_exr)):
        if q != 0. and p == 0.:
            zero_map[i] = q

    return zero_map.reshape((h, w, 1))

def run(filename1, filename2):
    return compare(
        pyexr.read(str(filename1)),
        pyexr.read(str(filename2))
    )

if __name__ == "__main__":
    print(run("/tmp/cbox-bw--debug/render_384_240.exr", "/tmp/cbox-bw--debug/batch_384_240.exr"))
    print(run("/tmp/cbox-bw--debug/render_20_134.exr", "/tmp/cbox-bw--debug/batch_20_134.exr"))

    print(run("/tmp/cbox-bw--debug/render_20_134.exr", "/tmp/cbox-bw--debug/batch_384_240.exr"))
    print(run("/tmp/cbox-bw--debug/render_384_240.exr", "/tmp/cbox-bw--debug/batch_20_134.exr"))
