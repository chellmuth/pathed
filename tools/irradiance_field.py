import math
import json

# This import registers the 3D projection, but is otherwise unused.
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 unused import

import matplotlib.pyplot as plt
import numpy as np

def normalized(x, y, z):
    length = math.sqrt(x**2 + y**2 + z**2)
    assert length != 0

    return (x / length, y / length, z / length)

def is_zero(color):
    r, g, b = color
    return r == 0 and g == 0 and b == 0

def location_plot(figure, sample):
    axes = figure.add_subplot(projection='3d')
    axes.set_title("Sample locations")

    query_x, query_y, query_z = sample["QueryPoint"]
    axes.scatter(query_x, query_z, query_y, marker="^")

    xs = []
    ys = []
    zs = []
    for photon in sample["Results"]:
        x, y, z = photon["point"]

        if is_zero(photon["throughput"]):
            continue

        xs.append(x)
        ys.append(y)
        zs.append(z)

    axes.scatter(xs, zs, ys)

    axes.set_xlabel("X")
    axes.set_ylabel("Z")
    axes.set_zlabel("Y")


def direction_plot(figure, sample):
    axes = figure.add_subplot(projection='3d')
    axes.set_title("Sample incoming directions")

    query_x, query_y, query_z = sample["QueryPoint"]
    axes.scatter(0, 0, 0, marker="^")

    xs = []
    ys = []
    zs = []
    for photon in sample["Results"]:
        x, y, z = photon["source"]

        if is_zero(photon["throughput"]):
            continue

        wi = normalized(
            x - query_x,
            y - query_y,
            z - query_z
        )

        xs.append(wi[0])
        ys.append(wi[1])
        zs.append(wi[2])

    axes.scatter(xs, zs, ys)

    axes.set_xlabel("X")
    axes.set_ylabel("Z")
    axes.set_zlabel("Y")


def importance_plot(figure, sample):
    phi_steps = 100
    theta_steps = 100
    grid = np.zeros((theta_steps, phi_steps, 3))

    print(len(sample["Results"]))
    for photon in sample["Results"]:
        origin_x, origin_y, origin_z = photon["point"]
        direction_x, direction_y, direction_z = photon["source"]

        wi_x, wi_y, wi_z = normalized(
            direction_x - origin_x,
            direction_y - origin_y,
            direction_z - origin_z
        )

        phi = np.arctan2(wi_z, wi_x)
        if phi < 0:
            phi += 2 * math.pi
        theta = np.arccos(wi_y)

        assert 0 <= phi <= 2 * math.pi
        assert 0 < theta <= math.pi

        phi_step = int(phi // (2 * math.pi / phi_steps))
        theta_step = int(theta // (math.pi / theta_steps))

        throughput = photon["throughput"]

        grid[theta_step, phi_step, :] += throughput

    axes = figure.add_subplot()

    axes.imshow(grid)
    axes.set_title("Photons")

    axes.set_xlabel("Phi")
    axes.set_ylabel("Theta")

    plt.show()

def run(sample):
    location_plot(plt.figure(1), sample)
    direction_plot(plt.figure(2), sample)
    importance_plot(plt.figure(3), sample)

    plt.show()


if __name__ == "__main__":
    run(json.load(open("../live-photons.json")))
