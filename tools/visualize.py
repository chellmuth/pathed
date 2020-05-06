import numpy as np
import pyexr
from matplotlib import cm, pyplot as plt

cmap = cm.magma

def sample_histogram(axes, sample_xs, sample_ys, bounds, bins=512):
    axes.hist2d(
        sample_xs, sample_ys,
        range=bounds, bins=bins, cmap=cmap, rasterized=False
    )
    axes.set_xlim(bounds[0])
    axes.set_ylim([1, 0])
    axes.set_xticks([])
    axes.set_yticks([])
    axes.set_aspect("equal")

def render_photon_grids(grids, output_path):
    figure, axes = plt.subplots(1, len(grids))

    if len(grids) == 1:
        axes = [axes]

    for i, grid in enumerate(grids):
        photon_bundle(axes[i], grid)

    plt.tight_layout()
    plt.savefig(output_path, bbox_inches='tight')
    plt.close()

def photon_bundle(axes, bundle):
    axes.imshow(
        bundle,
        origin="upper",
        cmap=cmap
    )
    axes.set_xticks([])
    axes.set_yticks([])
    axes.set_aspect("equal")

def density_mesh(axes, x, y, values, bounds, flip_y=False):
    if flip_y:
        values = np.flip(values, 0)

    axes.pcolormesh(
        x,
        y,
        values,
        cmap=cmap,
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

    density_mesh(axes, x, y, image[:, :, 0], bounds, flip_y=True)

    plt.tight_layout()
    plt.savefig(str(output_path), bbox_inches='tight')
    plt.close()
