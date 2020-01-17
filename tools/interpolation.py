import math
from dataclasses import dataclass

@dataclass
class Point:
    x: int
    y: int
    z: int

class Grid:
    def __init__(self, cells_x, cells_y, cells_z, data):
        self.cells_x = cells_x
        self.cells_y = cells_y
        self.cells_z = cells_z
        self.data = data

    def lookup(self, cell_x, cell_y, cell_z):
        if cell_x < 0 or cell_y < 0 or cell_z < 0: return 0
        if cell_x >= self.cells_x: return 0
        if cell_y >= self.cells_y: return 0
        if cell_z >= self.cells_z: return 0

        index = (cell_z * self.cells_y + cell_y) * self.cells_x + cell_x
        return self.data[index]

    def interpolate(self, x, y, z):
        x0 = math.floor(x)
        x1 = math.floor(x + 1)
        xd = (x - x0) / (x1 - x0)

        y0 = math.floor(y)
        y1 = math.floor(y + 1)
        yd = (y - y0) / (y1 - y0)

        z0 = math.floor(z)
        z1 = math.floor(z + 1)
        zd = (z - z0) / (z1 - z0)

        index000 = (x0, y0, z0)
        index001 = (x0, y0, z1)
        index010 = (x0, y1, z0)
        index011 = (x0, y1, z1)
        index100 = (x1, y0, z0)
        index101 = (x1, y0, z1)
        index110 = (x1, y1, z0)
        index111 = (x1, y1, z1)

        c000 = self.lookup(*index000)
        c001 = self.lookup(*index001)
        c010 = self.lookup(*index010)
        c011 = self.lookup(*index011)
        c100 = self.lookup(*index100)
        c101 = self.lookup(*index101)
        c110 = self.lookup(*index110)
        c111 = self.lookup(*index111)

        c_00 = c000 * (1 - xd) + c100 * xd
        c_01 = c001 * (1 - xd) + c101 * xd
        c_10 = c010 * (1 - xd) + c110 * xd
        c_11 = c011 * (1 - xd) + c111 * xd

        c__0 = c_00 * (1 - yd) + c_10 * yd
        c__1 = c_01 * (1 - yd) + c_11 * yd

        c = c__0 * (1 - zd) + c__1 * zd

        return c
