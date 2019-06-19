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

def location_plot(figure, sample):
    axes = figure.add_subplot(projection='3d')

    query_x, query_y, query_z = sample["QueryPoint"]
    axes.scatter(query_x, query_z, query_y, marker="^")

    xs = []
    ys = []
    zs = []
    for photon in sample["Results"]:
        x, y, z = photon["point"]
        xs.append(x)
        ys.append(y)
        zs.append(z)

    axes.scatter(xs, zs, ys)

    axes.set_xlabel("X")
    axes.set_ylabel("Z")
    axes.set_zlabel("Y")

def direction_plot(figure, sample):
    axes = figure.add_subplot(projection='3d')

    query_x, query_y, query_z = sample["QueryPoint"]
    axes.scatter(0, 0, 0, marker="^")

    xs = []
    ys = []
    zs = []
    for photon in sample["Results"]:
        x, y, z = photon["source"]

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

def run(sample):
    location_plot(plt.figure(1), sample)
    direction_plot(plt.figure(2), sample)

    plt.show()

# # unit area ellipse
# rx, ry = 3., 1.
# area = rx * ry * np.pi
# theta = np.arange(0, 2 * np.pi + 0.01, 0.1)
# verts = np.column_stack([rx / area * np.cos(theta), ry / area * np.sin(theta)])

# x, y, s, c = np.random.rand(4, 30)
# s *= 10**2.

# fig, ax = plt.subplots()
# ax.scatter(x, y, s, c, marker=verts)

# plt.show()


if __name__ == "__main__":
    run(json.load(open("floor.json")))
