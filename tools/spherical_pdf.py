import math
import random

import numpy as np

phi_cells = 20
theta_cells = 20

phi_steps = 100
theta_steps = 100

x_steps = 100
y_steps = 100

def cartesian_to_spherical(x, y, z):
    phi = math.atan2(z, x)
    if phi < 0.0:
        phi += math.pi * 2

    if phi == math.pi * 2:
        phi = 0.0

    theta = math.acos(y)

    return phi, theta

def run():
    grid = np.random.rand(phi_cells, theta_cells)
    grid = grid / np.sum(grid)

    d_phi = (2 * math.pi) / phi_steps
    d_theta = (math.pi / 2) / theta_steps

    estimator = 0
    for phi_step in range(phi_steps):
        for theta_step in range(theta_steps):
            phi_cell = int(phi_cells * ((phi_step + 0.5) / phi_steps))
            theta_cell = int(theta_cells * ((theta_step + 0.5) / theta_steps))

            mass = grid[phi_cell][theta_cell]

            phi1 = (2 * math.pi) * phi_cell / phi_cells
            phi2 = (2 * math.pi) * (phi_cell + 1) / phi_cells

            theta1 = (math.pi / 2) * theta_cell / theta_cells
            theta2 = (math.pi / 2) * (theta_cell + 1) / theta_cells

            pdf = mass / ((math.cos(theta1) - math.cos(theta2)) * (phi2 - phi1))

            step_theta = (math.pi / 2) * (theta_step + 0.5) / theta_steps

            estimator += pdf * math.sin(step_theta) * d_phi * d_theta

    cartesian_estimator = 0
    for x_step in range(x_steps):
        for y_step in range(y_steps):
            x = (x_step + 0.5) / x_steps
            y = (y_step + 0.5) / y_steps
            z = math.sqrt(1.0 - x**2 - y**2)

            theta = x * math.pi * 2
            phi_cell = int(phi_cells * ((phi_step + 0.5) / phi_steps))
            theta_cell = int(theta_cells * ((theta_step + 0.5) / theta_steps))

            mass = grid[phi_cell][theta_cell]

            phi1 = (2 * math.pi) * phi_cell / phi_cells
            phi2 = (2 * math.pi) * (phi_cell + 1) / phi_cells

            theta1 = (math.pi / 2) * theta_cell / theta_cells
            theta2 = (math.pi / 2) * (theta_cell + 1) / theta_cells

            pdf = mass / ((math.cos(theta1) - math.cos(theta2)) * (phi2 - phi1))

            step_theta = (math.pi / 2) * (theta_step + 0.5) / theta_steps

            estimator += pdf * math.sin(step_theta) * d_phi * d_theta


    monte_carlo_samples = 100000
    monte_carlo_estimator = 0
    for _ in range(monte_carlo_samples):
        y = random.random()

        phi = math.pi * 2 * random.random()
        theta = math.acos(y)

        phi_cell = int((phi / (2 * math.pi)) / phi_cells)
        theta_cell = int((theta / (math.pi / 2)) * theta_cells)

        assert(phi_cell < phi_cells)
        assert(theta_cell < theta_cells)

        mass = grid[phi_cell][theta_cell]

        phi1 = (2 * math.pi) * phi_cell / phi_cells
        phi2 = (2 * math.pi) * (phi_cell + 1) / phi_cells

        theta1 = (math.pi / 2) * theta_cell / theta_cells
        theta2 = (math.pi / 2) * (theta_cell + 1) / theta_cells

        pdf = mass / ((math.cos(theta1) - math.cos(theta2)) * (phi2 - phi1))

        monte_carlo_estimator += pdf * 2 * math.pi / monte_carlo_samples

    print(estimator)
    print(monte_carlo_estimator)

if __name__ == "__main__":
    run()
