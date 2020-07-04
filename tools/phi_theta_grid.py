import itertools
import math
import struct

import numpy as np

import coordinates
from transform import Transform
from vector import Vector

class PhotonGridAdapter:
    def __init__(self, position, normal, wi):
        self.position = position
        self.normal = normal
        self.wi = wi

        self.to_tangent = Transform.to_tangent(self.normal, self.wi)
        self.to_world = Transform.to_world(self.normal, self.wi)

    def splat_params(self, photon_data):
        direction_world = (photon_data.source - self.position).normalized()
        direction = self.to_tangent.transform_direction(direction_world)

        phi, theta = coordinates.cartesian_to_spherical(direction)
        power = sum(photon_data.power) / 3.
        depth = photon_data.depth

        return phi, theta, power, depth


class PhotonRepresentation:
    def splat(self, phi, theta, value):
        raise NotImplementedError()

    def export_dat(self, output_path):
        raise NotImplementedError()

class FatPhotonDataset(PhotonRepresentation):
    def __init__(self):
        self.photons = []
        self.values = []
        self.depths = []

    def splat(self, phi, theta, value, depth):
        # if theta > math.pi / 2.:
        #     return False

        self.photons.append((
            phi / (2. * math.pi) - 0.5,
            theta / (math.pi / 2.) - 0.5,
        ))
        self.values.append(value)
        self.depths.append(depth)

        return True

    def export_dat(self, output_path):
        output_file = open(str(output_path), mode="wb")

        count = len(self.photons)
        # count_data = struct.pack("d", count)
        # output_file.write(count_data)

        normalized_values = []
        length = np.sum(self.values)
        for value in self.values:
            normalized_values.append(value / length)

        merged = []
        for photon, value, depth in zip(self.photons, normalized_values, self.depths):
            merged.append((photon[0], photon[1], value, depth))

        if count > 0:
            float_data = struct.pack(
                f"{count * len(merged[0])}f",
                *list(itertools.chain(*merged))
            )

            output_file.write(float_data)

        output_file.close()

class PhiThetaGrid(PhotonRepresentation):
    def __init__(self, phi_steps, theta_steps):
        self.phi_steps = phi_steps
        self.theta_steps = theta_steps

        self.length = self.phi_steps * self.theta_steps

        self.grid = [ 0. for _ in range(self.length) ]

    def _theta_step(self, theta):
        return int((theta / (math.pi / 2.)) * self.theta_steps)

    def _phi_step(self, phi):
        return int((phi / (2. * math.pi)) * self.phi_steps)

    def _index_stepped(self, phi_step, theta_step):
        return self.phi_steps * theta_step + phi_step

    def _index(self, phi, theta):
        phi_step = self._phi_step(phi)
        theta_step = self._theta_step(theta)

        return self._index_stepped(phi_step, theta_step)

    def splat(self, phi, theta, value, depth):
        index = self._index(phi, theta)

        if 0 <= index < self.length:
            self.grid[index] += value
            return True

        return False

    def export_dat(self, output_path):
        grid_sum = sum(self.grid)
        if grid_sum == 0:
            pdf = [ 0 for cell in self.grid ]
        else:
            pdf = [ cell / grid_sum for cell in self.grid ]

        output_file = open(str(output_path), mode="wb")
        float_data = struct.pack(f"{self.length}f", *pdf)
        output_file.write(float_data)
        output_file.close()

    def raw_grid(self):
        grid_sum = sum(self.grid)
        if grid_sum == 0:
            pdf = [ 0 for cell in self.grid ]
        else:
            pdf = [ cell / grid_sum for cell in self.grid ]

        raw_grid = np.array(pdf).reshape((self.phi_steps, self.theta_steps))
        return raw_grid
