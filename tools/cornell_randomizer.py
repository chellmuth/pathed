import json
import math
import random
from pathlib import Path

import numpy as np

def get_points():
    points = np.array([
        ( 0.53, 0.60, 0.75, 1),
        ( 0.70, 0.60, 0.17, 1),
        ( 0.13, 0.60, 0.00, 1),
        (-0.05, 0.60, 0.57, 1),
        (-0.05, 0.00, 0.57, 1),
        (-0.05, 0.60, 0.57, 1),
        ( 0.13, 0.60, 0.00, 1),
        ( 0.13, 0.00, 0.00, 1),
        ( 0.53, 0.00, 0.75, 1),
        ( 0.53, 0.60, 0.75, 1),
        (-0.05, 0.60, 0.57, 1),
        (-0.05, 0.00, 0.57, 1),
        ( 0.70, 0.00, 0.17, 1),
        ( 0.70, 0.60, 0.17, 1),
        ( 0.53, 0.60, 0.75, 1),
        ( 0.53, 0.00, 0.75, 1),
        ( 0.13, 0.00, 0.00, 1),
        ( 0.13, 0.60, 0.00, 1),
        ( 0.70, 0.60, 0.17, 1),
        ( 0.70, 0.00, 0.17, 1),
        ( 0.53, 0.00, 0.75, 1),
        ( 0.70, 0.00, 0.17, 1),
        ( 0.13, 0.00, 0.00, 1),
        (-0.05, 0.00, 0.57, 1),
    ])
    return points

def scale(x, y, z):
    return np.array([
        [ x, 0, 0, 0, ],
        [ 0, y, 0, 0, ],
        [ 0, 0, z, 0, ],
        [ 0, 0, 0, 1, ],
    ])

def translate(dx, dy, dz):
    return np.array([
        [ 1, 0, 0, dx, ],
        [ 0, 1, 0, dy, ],
        [ 0, 0, 1, dz, ],
        [ 0, 0, 0,  1, ],
    ])

def rotate_x(theta):
    return np.array([
        [ 1,               0,                0, 0, ],
        [ 0, math.cos(theta), -math.sin(theta), 0, ],
        [ 0, math.sin(theta),  math.cos(theta), 0, ],
        [ 0,               0,                0, 1, ],
    ])

def rotate_y(theta):
    return np.array([
        [  math.cos(theta),  0, math.sin(theta), 0, ],
        [                0,  1,               0, 0, ],
        [ -math.sin(theta),  0, math.cos(theta), 0, ],
        [                0,  0,               0, 1, ],
    ])

def identity():
    return np.array([
        [ 1, 0, 0, 0, ],
        [ 0, 1, 0, 0, ],
        [ 0, 0, 1, 0, ],
        [ 0, 0, 0, 1, ],
    ])

def rand_over(low, hi):
    distance = hi - low
    return low + random.random() * distance

def box_center(points):
    means = np.mean(points, axis=0)
    return means[0:3]

def perturb_and_write(f):
    points = get_points()

    center = box_center(points)

    transforms = [
        translate(-center[0], -center[1], -center[2]),
        scale(
            rand_over(0.5, 1.5),
            rand_over(0.8, 2),
            rand_over(0.8, 2)
        ),
        rotate_x(2 * math.pi * random.random()),
        rotate_y(2 * math.pi * random.random()),
        translate(center[0], center[1], center[2]),
        translate(
            rand_over(-0.2, 0.2),
            rand_over(0, 0.5),
            rand_over(-0.2, 0.2)
        ),
    ]

    transform = identity()
    for t in transforms:
        transform = t.dot(transform)

    result = transform.dot(points.transpose()).transpose()

    f.write(cornell_pt1())

    faces = [
        "f -4 -3 -2 -1",
        "f -4 -3 -2 -1",
        "f -4 -3 -2 -1",
        "f -4 -3 -2 -1",
        "f -4 -3 -2 -1",
        "f -12 -11 -10 -9",
    ]

    for i in range(len(points) // 4):
        for j in range(4):
            x, y, z, _ = result[4 * i + j]
            f.write(f"v {x} {y} {z}\n")
        f.write(faces[i] + "\n")
        f.write("\n")

    f.write(cornell_pt2())

def one(i):
    procedural_path_local = Path("..") / "procedural"
    procedural_path_runtime = Path(".") / "procedural"

    obj_filename = f"CornellBox-{i:04d}.obj"
    obj_path = procedural_path_local / obj_filename

    scene_filename = f"cornell-{i:04d}.json"
    print(obj_path, scene_filename)

    with open(obj_path, "w") as f:
        perturb_and_write(f)

    with open(procedural_path_local / scene_filename, "w") as f:
        f.write(scene_content(procedural_path_runtime / obj_filename))

def go(count):
    for i in range(count):
        one(i)

def cornell_pt1():
    return """
# The original Cornell Box in OBJ format.
# Note that the real box is not a perfect cube, so
# the faces are imperfect in this data set.
#
# Created by Guedis Cardenas and Morgan McGuire at Williams College, 2011
# Released into the Public Domain.
#
# http://graphics.cs.williams.edu/data
# http://www.graphics.cornell.edu/online/box/data.html
#

mtllib CornellBox-Original.mtl

## Object floor
v  -1.01  0.00   0.99
v   1.00  0.00   0.99
v   1.00  0.00  -1.04
v  -0.99  0.00  -1.04

g floor
usemtl floor
f -4 -3 -2 -1

## Object ceiling
v  -1.02  1.99   0.99
v  -1.02  1.99  -1.04
v   1.00  1.99  -1.04
v   1.00  1.99   0.99

g ceiling
usemtl ceiling
f -4 -3 -2 -1

## Object backwall
v  -0.99  0.00  -1.04
v   1.00  0.00  -1.04
v   1.00  1.99  -1.04
v  -1.02  1.99  -1.04

g backWall
usemtl backWall
f -4 -3 -2 -1

## Object rightwall
v	1.00  0.00  -1.04
v	1.00  0.00   0.99
v	1.00  1.99   0.99
v	1.00  1.99  -1.04

g rightWall
usemtl rightWall
f -4 -3 -2 -1

## Object leftWall
v  -1.01  0.00   0.99
v  -0.99  0.00  -1.04
v  -1.02  1.99  -1.04
v  -1.02  1.99   0.99

g leftWall
usemtl leftWall
f -4 -3 -2 -1

## Object shortBox
usemtl shortBox
"""

def cornell_pt2():
    return """
g shortBox
usemtl shortBox

## Object tallBox
usemtl tallBox

# Top Face
v	-0.53  1.20   0.09
v	 0.04  1.20  -0.09
v	-0.14  1.20  -0.67
v	-0.71  1.20  -0.49
f -4 -3 -2 -1

# Left Face
v	-0.53  0.00   0.09
v	-0.53  1.20   0.09
v	-0.71  1.20  -0.49
v	-0.71  0.00  -0.49
f -4 -3 -2 -1

# Back Face
v	-0.71  0.00  -0.49
v	-0.71  1.20  -0.49
v	-0.14  1.20  -0.67
v	-0.14  0.00  -0.67
f -4 -3 -2 -1

# Right Face
v	-0.14  0.00  -0.67
v	-0.14  1.20  -0.67
v	 0.04  1.20  -0.09
v	 0.04  0.00  -0.09
f -4 -3 -2 -1

# Front Face
v	 0.04  0.00  -0.09
v	 0.04  1.20  -0.09
v	-0.53  1.20   0.09
v	-0.53  0.00   0.09
f -4 -3 -2 -1

# Bottom Face
v	-0.53  0.00   0.09
v	 0.04  0.00  -0.09
v	-0.14  0.00  -0.67
v	-0.71  0.00  -0.49
f -8 -7 -6 -5

g tallBox
usemtl tallBox

## Object light
v	-0.24  1.98   0.16
v	-0.24  1.98  -0.22
v	 0.23  1.98  -0.22
v	 0.23  1.98   0.16

g light
usemtl light
f -4 -3 -2 -1
"""

def scene_content(obj_path):
    json_content = {
        "sensor": {
            "lookAt": {
                "origin": [ "0", "1", "6.8" ],
                "target": [ "0", "1", "0" ],
                "up": [ "0", "1", "0" ]
            },
            "fov": "19.5"
        },
        "models": [
            {
                "type": "obj",
                "filename": str(obj_path)
            }
        ]
    }

    return json.dumps(json_content, indent=2) + "\n"


if __name__ == "__main__":
    go()
