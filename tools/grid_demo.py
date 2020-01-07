import math
from dataclasses import dataclass

import click

@dataclass
class Point:
    x: float
    y: float

def length(point1, point2):
    return math.sqrt((point2.x - point1.x) ** 2 + (point2.y - point1.y) ** 2)

def calc_next_distance(current, is_forward):
    if is_forward:
        if current == int(current):
            return current + 1.
        return math.ceil(current) - current
    else:
        if current == int(current):
            return current - 1.
        return math.floor(current) - current

def calc_next_time(rate, entry_point):
    next_distance = calc_next_distance(entry_point, rate > 0.)
    return next_distance / rate

def cell(current_point):
    return Point(math.floor(current_point.x), math.floor(current_point.y))

def last_cell(exit_point):
    guess = cell(exit_point)
    if exit_point.x == int(exit_point.x):
        guess.x -= 1

    if exit_point.y == int(exit_point.y):
        guess.y -= 1

    return guess

@click.command()
def init():
    x_cols = 2
    y_cols = 2

    entry_point = Point(1.8, 0.)
    exit_point = Point(0.1, 1.1)

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

    next_times = Point(
        calc_next_time(rates.x, entry_point.x),
        calc_next_time(rates.y, entry_point.y)
    )

    current_cell = cell(entry_point)
    print("last cell:", last_cell(exit_point))

    print("visited cell:", current_cell)
    print("initial next times:", next_times)

    while current_cell != last_cell(exit_point):
        if next_times.x < next_times.y:
            if rates.x > 0:
                next_times.x += 1. / rates.x
                current_cell.x += 1
            else:
                next_times.x -= 1. / rates.x
                current_cell.x -= 1
        else:
            if rates.y > 0:
                next_times.y += 1. / rates.y
                current_cell.y += 1
            else:
                next_times.y -= 1. / rates.y
                current_cell.y -= 1

        print("visited cell:", current_cell)
        print(next_times)

if __name__ == "__main__":
    init()
