import json

import matplotlib.pyplot as plt
import numpy as np
import pyexr

def _srgb(grid):
    grid = np.where(
        grid <= 0.0031308,
        12.92 * grid,
        1.055 * grid ** (1 / 2.4) - 0.055
    )

    return np.clip(grid, 0.0, 1.0)

def _make_grid(gt):
    phi_steps = gt["Steps"]["phi"]
    theta_steps = gt["Steps"]["theta"]

    grid = np.zeros((theta_steps, phi_steps, 3))

    for i, sample in enumerate(gt["Gt"]):
        phi_step = sample["phiStep"]
        theta_step = sample["thetaStep"]

        grid[theta_step, phi_step, :] = sample["radiance"]

    return grid

def run(gt):
    grid = _make_grid(gt)

    pyexr.write("live-gt.exr", grid)

    grid = _srgb(grid)

    figure = plt.figure()
    axes = figure.add_subplot()

    axes.imshow(grid)
    axes.set_title("Gt Sampling")

    axes.set_xlabel("Phi")
    axes.set_ylabel("Theta")

    plt.show()


if __name__ == "__main__":
    run(json.load(open("../live-gt.json")))
