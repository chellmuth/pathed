import math

import numpy as np

phi_cells = 20
theta_cells = 20

phi_steps = 100
theta_steps = 100

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

    print(estimator)

if __name__ == "__main__":
    run()
