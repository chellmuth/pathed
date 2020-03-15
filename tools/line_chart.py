import json

from matplotlib import pyplot as plt

def load(filename):
    return json.load(open(filename))

def chart(data):
    figure = plt.figure()

    axes = figure.add_subplot()

    for line in data:
        xs = [ d["x"] for d in line["data"] ]
        ys = [ d["y"] for d in line["data"] ]
        axes.plot(xs, ys)

    plt.legend([d["legend"] for d in data])

    axes.axhline(color="black", linewidth=1.)
    axes.axvline(color="black", linewidth=1.)

    plt.show()

def vectors(data):
    figure = plt.figure()

    axes = figure.add_subplot()
    axes.set_xlim((-1, 1))
    axes.set_ylim((-1, 1))

    for line in data:
        d = line["data"]
        xs = [ d["incident"][0], 0., d["transmitted"][0] ]
        ys = [ d["incident"][1], 0., d["transmitted"][1] ]
        axes.plot(xs, ys)

    plt.legend([d["legend"] for d in data])
    plt.show()

if __name__ == "__main__":
    # data = load("../Release/testbed.json")
    # chart(data["fresnel"])
    # vectors(data["snell"])

    data = load("../Release/testbed-microfacet.json")
    chart(data["D"])
