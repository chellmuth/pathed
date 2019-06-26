from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import pyexr

def calculate_mse(test, gt):
    return np.sum((gt - test) ** 2)

def calculate_ae(test, gt):
    return np.sum(np.abs(gt - test))

def build_dataset(error_fn, gt, data_directory):
    mses = []

    i = 0
    while True:
        exr_path = Path(f"../{data_directory}/auto-{2**i:05d}spp.exr")
        if not exr_path.exists():
            break

        mse = error_fn(
            pyexr.read(str(exr_path)),
            gt
        )
        mses.append(mse)

        i += 1

    return mses

def build_datasets(error_fn, gt, plots):
    return [
        build_dataset(error_fn, gt, data_directory)
        for _, data_directory
        in plots
    ]

@ticker.FuncFormatter
def power_of_two_formatter(x, pos):
    return int(2 ** x)

def run(gt, plots):
    errors = [ ("MSE", calculate_mse), ("AE", calculate_ae) ]

    fig, axs = plt.subplots(nrows=len(errors))
    fig.suptitle("Convergence Plots")

    for error, ax in zip(errors, axs):
        error_name, error_fn = error

        ys_list = build_datasets(error_fn, gt, plots)
        for i, ys in enumerate(ys_list):
            xs = list(range(len(ys)))
            ax.plot(xs, ys, label=plots[i][0])

            ax.set(xlabel='spp (log)', ylabel=error_name)
            ax.xaxis.set_major_formatter(power_of_two_formatter)

            ax.legend(loc='upper right')

    plt.tight_layout(rect=[0, 0, 1, 0.95]) # rect fixes suptitle clipping
    plt.show()

if __name__ == "__main__":
    gt = pyexr.read("../path-traced/auto-16384spp.exr")

    run(gt, [("Path Traced", "path-traced"), ("Ours", "pdf-test"), ("Ours-Sanity", "default-output")])
