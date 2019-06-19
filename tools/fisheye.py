import json

import matplotlib.pyplot as plt
import numpy as np

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

    figure = plt.figure()
    axes = figure.add_subplot()

    axes.imshow(grid)
    axes.set_title("Gt Sampling")

    axes.set_xlabel("Phi")
    axes.set_ylabel("Theta")

    plt.show()


if __name__ == "__main__":
    run(json.load(open("../live-gt.json")))
