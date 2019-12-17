import json

from matplotlib import pyplot as plt

def load(filename):
    return json.load(open(filename))

def chart(data):
    figure = plt.figure(1)

    axes = figure.add_subplot()

    xs = [ d["x"] for d in data ]
    ys = [ d["y"] for d in data ]
    axes.plot(xs, ys)

    plt.show()

if __name__ == "__main__":
    data = load("../Release/testbed-fresnel.json")
    chart(data)
