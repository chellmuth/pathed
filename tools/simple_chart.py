from matplotlib import pyplot as plt

def scatter(xs, ys, title="", x_label="", y_label=""):
    figure = plt.figure()

    axes = figure.add_subplot()
    axes.set_title(title)
    axes.set_xlabel(x_label)
    axes.set_ylabel(y_label)

    axes.scatter(xs, ys)

    plt.show()

if __name__ == "__main__":
    xs = [0.17779753325689862, 0.324078025714716, 0.3168952456762951, 1.7288042156862395, 1.3214101247048755, 4.604378658180982, 2.5206778689700453, 0.32172072495499127, 0.7993905075039794, 0.25070604628211796]
    ys = [0.0008517259875701009, 0.008272632398635042, 0.00034033572651171866, 0.0025949456990287576, 0.0036079609906492617, 0.00020368578596077135, 0.0002631714885344037, 0.0008328485706839149, 0.008909678498539835, 0.0025892975802712724]

    scatter(xs, ys)


    ys = [
        0.00021250203004447607,
        0.005656487657946681,
        0.002731739244288402,
        0.002781616089174393,
        0.014174148964625854,
        0.0032235445665476973,
        0.0009887280815972293,
        0.036016884265256205,
        0.057482499419288596,
        0.00011122185354465937,
    ]
    scatter(xs, ys)