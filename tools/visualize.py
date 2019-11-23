import numpy as np
import pyexr
from matplotlib import cm, pyplot as plt

cmap = cm.magma

def photon_bundle(axes, bundle):
    axes.imshow(
        bundle,
        origin="lower",
        cmap=cmap
    )
    axes.set_xticks([])
    axes.set_yticks([])
    axes.set_aspect("equal")

def density_mesh(axes, x, y, values, bounds):
    axes.pcolormesh(
        x,
        y,
        values,
        cmap=cmap
    )

    axes.set_xlim(bounds[0])
    axes.set_ylim(bounds[1])
    axes.set_xticks([])
    axes.set_yticks([])
    axes.set_aspect("equal")

def convert_to_density_mesh(input_path, output_path):
    num_points_per_axis = 400
    bounds = np.array([
        [0, 1],
        [0, 1]
    ])

    x = np.linspace(bounds[0][0], bounds[0][1], num_points_per_axis)
    y = np.linspace(bounds[1][0], bounds[1][1], num_points_per_axis)

    figure, axes = plt.subplots(1, 1)

    image = pyexr.read(str(input_path))

    density_mesh(axes, x, y, image[:, :, 0], bounds)

    plt.tight_layout()
    plt.savefig(str(output_path), bbox_inches='tight')
    plt.close()
