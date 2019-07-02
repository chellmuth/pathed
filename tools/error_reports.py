import json
import math
import sys
from pathlib import Path

import click
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import pyexr

def calculate_mse(test, gt):
    h, w, _ = gt.shape
    return np.sum((gt - test) ** 2) / (h * w)

def calculate_ae(test, gt):
    h, w, _ = gt.shape
    return np.sum(np.abs(gt - test)) / (h * w)

def build_dataset(error_fn, gt, data_path):
    mses = []

    i = 0
    while True:
        exr_path = data_path / f"auto-{2**i:05d}spp.exr"
        if not exr_path.exists():
            break

        mse = error_fn(
            pyexr.read(str(exr_path)),
            gt
        )
        mses.append(mse)

        i += 1

    return mses

def build_datasets(error_fn, gt, data_sources):
    return [
        build_dataset(error_fn, gt, data_path)
        for _, data_path
        in data_sources
    ]

@ticker.FuncFormatter
def power_of_two_formatter(x, pos):
    return int(2 ** x)

def run(gt, data_sources, max_spp):
    errors = [ ("MSE", calculate_mse), ("AE", calculate_ae) ]

    fig, axs = plt.subplots(nrows=len(errors))
    fig.suptitle("Convergence Plots")

    for error, ax in zip(errors, axs):
        error_name, error_fn = error

        ys_list = build_datasets(error_fn, gt, data_sources)
        for i, ys in enumerate(ys_list):
            xs = list(range(len(ys)))

            if max_spp:
                max_index = int(math.log2(max_spp)) + 1
                xs = xs[:max_index]
                ys = ys[:max_index]

            ax.plot(xs, ys, label=data_sources[i][0])

            ax.set(xlabel='spp (log)', ylabel=error_name)
            ax.xaxis.set_major_formatter(power_of_two_formatter)

            ax.legend(loc='upper right')

    plt.tight_layout(rect=[0, 0, 1, 0.95]) # rect fixes suptitle clipping
    plt.show()

def find_data_sources(root_path, includes, scene):
    if includes:
        json_paths = [
            Path(p) / "report.json"
            for p in includes
        ]
    else:
        json_paths = root_path.glob("*/report.json")

    data_sources = []
    for json_path in json_paths:
        report_json = json.load(open(json_path))
        if report_json["scene"] == scene + ".json":
            data_sources.append((report_json["output_name"], json_path.parent))

    return data_sources

@click.command()
@click.argument("scene")
@click.option("--gt", type=click.Path(exists=True))
@click.option("--includes", multiple=True)
@click.option("--max-spp", default=None, type=int)
def init(scene, gt, includes, max_spp):
    if gt:
        gt = pyexr.read(gt)
    else:
        gt = pyexr.read(f"{scene}-gt.exr")

    data_sources = find_data_sources(Path(".."), includes, scene)
    if not data_sources:
        print("No data sources found!")
        sys.exit(1)

    for data_source in data_sources:
        print(data_source)

    run(gt, data_sources, max_spp)


if __name__ == "__main__":
    init()
