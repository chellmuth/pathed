import math

import coordinates
from vector import Vector

class PhotonGridAdapter:
    def __init__(self, position):
        self.position = position

    def splat_params(self, photon_data):
        direction = (photon_data.source - self.position).normalized()

        phi, theta = coordinates.cartesian_to_spherical(direction)
        power = sum(photon_data.power) / 3.

        return phi, theta, power


class PhiThetaGrid:
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

    def splat(self, phi, theta, value):
        index = self._index(phi, theta)

        self.grid[index] += value
