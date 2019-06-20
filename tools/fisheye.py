import json

import matplotlib.pyplot as plt
import numpy as np

def convert(color):
    return np.clip([srgb(c) for c in color], 0.0, 1.0)

def srgb(channel):
    if channel <= 0.0031308:
        return 12.92 * channel
    return 1.055 * channel ** (1 / 2.4) - 0.055

def _make_grid(gt):
    phi_steps = gt["Steps"]["phi"]
    theta_steps = gt["Steps"]["theta"]

    grid = np.zeros((theta_steps, phi_steps, 3))

    for i, sample in enumerate(gt["Gt"]):
        phi_step = sample["phiStep"]
        theta_step = sample["thetaStep"]

        grid[theta_step, phi_step, :] = convert(sample["radiance"])

    return grid

def run(gt):
    grid = _make_grid(gt)

    figure = plt.figure()
    axes = figure.add_subplot()

    axes.imshow(grid)
    axes.set_title("Gt Sampling")

    axes.set_xlabel("Phi")
    axes.set_ylabel("Theta")

    plt.show()


if __name__ == "__main__":
    run(json.load(open("../live-gt.json")))
