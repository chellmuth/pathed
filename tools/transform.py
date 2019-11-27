import numpy as np

from vector import Vector

class Transform:
    def __init__(self, data):
        self.data = np.array(data)

    @classmethod
    def to_tangent(cls, normal: Vector, wi: Vector):
        return cls.to_world(normal, wi).transposed()

    @classmethod
    def to_world(cls, normal: Vector, wi: Vector):
        x_axis = normal.cross(wi).normalized()
        z_axis = normal.cross(x_axis).normalized()

        return cls([
            [ x_axis.x, normal.x, z_axis.x, 0. ],
            [ x_axis.y, normal.y, z_axis.y, 0. ],
            [ x_axis.z, normal.z, z_axis.z, 0. ],
            [ 0., 0., 0., 1. ],
        ])

    def transposed(self):
        return Transform(self.data.T)

    def transform_direction(self, direction: Vector):
        x = np.array([direction.x, direction.y, direction.z, 0.])

        result = self.data.dot(x)
        return Vector(*self.data.dot(x)[:3])

    # def transform_point(self, point: Vector):
    #     x = np.array([point.x, point.y, point.z, 1.])
    #     return self.data.dot(x)
