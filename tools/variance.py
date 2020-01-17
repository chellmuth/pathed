import json
import struct

import numpy as np

def sample_mean(samples):
    return np.mean(samples)

def sample_variance(samples):
    mean = sample_mean(samples)
    return sum(
        (sample - mean) ** 2
        for sample in samples
    ) / (len(samples) - 1)

def mrse(samples, gt):
    measurement = sample_mean(samples)
    return (measurement - gt) ** 2 / gt

def ae(samples, gt):
    measurement = sample_mean(samples)
    return abs(measurement - gt)

def mse(samples, gt):
    measurement = sample_mean(samples)
    return (measurement - gt) ** 2

def relative_bias(samples, gt):
    measurement = sample_mean(samples)
    return abs(measurement - gt) / gt

def errors(samples, count, gt):
    return {
        "ae": ae(samples[:count], gt),
        "mrse": mrse(samples[:count], gt),
        "mse": mse(samples[:count], gt),
        "variance": sample_variance(samples[:count]),
        "bias": relative_bias(samples[:count], gt),
        "mean": sample_mean(samples[:count]),
        "gt": gt,
    }

def read_bin(filename):
    with open(filename, "rb") as f:
        count, = struct.unpack("i", f.read(4))
        variances = struct.unpack(f"{count}f", f.read(4 * count))
        # print(count)
        # print(variances)

        return np.array(variances)

if __name__ == "__main__":
    # read_bin("/home/cjh/workpad/src/mitsuba/samples_20_134.bin")

    # samples = np.array(json.load(open("samples.json")))
    # print(sample_variance(samples))

    points = [
        (20, 134), # left wall
        (268, 151), # back wall
        (384, 240), # right wall
        (114, 26), # ceiling
        (313, 349), # short box - right side
        (246, 315), # short box - front side
        (228, 270), # short box - top side
        (82, 388), # floor
        (146, 210), # tall box - front side
        (94, 175), # tall box - left side
    ]

    for x, y in points:
        filename = f"/tmp/cbox-bw--debug/samples_{x}_{y}.bin"
        samples = read_bin(filename)

        print(mrse(samples, 1))
        # print(mse(samples, 4))
        # print(ae(samples, 4))
