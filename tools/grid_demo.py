import math
from dataclasses import dataclass

import click

@dataclass
class Point:
    x: float
    y: float

def length(point1, point2):
    return math.sqrt((point2.x - point1.x) ** 2 + (point2.y - point1.y) ** 2)

def calc_next_distance(current):
    if current == int(current):
        return current + 1.
    return math.ceil(current) - current

def calc_next_time(rate, next_distance):
    return next_distance / rate

def cell(current_point):
    return (math.floor(current_point.x), math.floor(current_point.y))

def last_cell(exit_point):
    guess = cell(exit_point)
    if exit_point.x == int(exit_point.x):
        guess = (guess[0] - 1, guess[1])

    if exit_point.y == int(exit_point.y):
        guess = (guess[0], guess[1] - 1)

    return guess

@click.command()
def init():
    x_cols = 2
    y_cols = 2

    entry_point = Point(0., 0.)
    exit_point = Point(2., 1.1)

    spans = Point(
        exit_point.x - entry_point.x,
        exit_point.y - entry_point.y,
    )

    distance = length(entry_point, exit_point)
    print("distance:", distance)

    rates = Point(
        spans.x / distance,
        spans.y / distance,
    )

    print("rates:", rates)

    current_positions = Point(
        entry_point.x,
        entry_point.y
    )

    next_distances = Point(
        calc_next_distance(current_positions.x),
        calc_next_distance(current_positions.y)
    )

    next_times = Point(
        calc_next_time(rates.x, next_distances.x),
        calc_next_time(rates.y, next_distances.y),
    )

    current_cell = cell(current_positions)
    print("last cell:", last_cell(exit_point))

    print("visited cell:", current_cell)
    print("initial next times:", next_times)

    while current_cell != last_cell(exit_point):
        if next_times.x < next_times.y:
            next_times.x += 1. / rates.x
            current_cell = (current_cell[0] + 1, current_cell[1])
        else:
            next_times.y += 1. / rates.y
            current_cell = (current_cell[0], current_cell[1] + 1)

        print("visited cell:", current_cell)
        print(next_times)

if __name__ == "__main__":
    init()
