import math

class Vector:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def length(self):
        return math.sqrt(
            self.x ** 2 \
            + self.y ** 2 \
            + self.z ** 2
        )

    def normalized(self):
        return self / self.length()

    def dot(self, other):
        return self.x * other.x \
            + self.y * other.y \
            + self.z * other.z

    def cross(self, other):
        return Vector(
            self.y * other.z - self.z * other.y,
            self.z * other.x - self.x * other.z,
            self.x * other.y - self.y * other.x
        )

    def __sub__(self, other):
        return Vector(
            self.x - other.x,
            self.y - other.y,
            self.z - other.z
        )

    def __truediv__(self, other):
        return Vector(
            self.x / other,
            self.y / other,
            self.z / other
        )

    def __repr__(self):
        return f"({self.x:f}, {self.y:f}, {self.z:f})"
