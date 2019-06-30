import math

import numpy as np

phi_cells = 1
theta_cells = 1

phi_steps = 2
theta_steps = 2

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

            phi1 = (2 * math.pi) * phi_step / phi_steps
            phi2 = (2 * math.pi) * (phi_step + 1) / phi_steps

            theta1 = (math.pi / 2) * theta_step / theta_steps
            theta2 = (math.pi / 2) * (theta_step + 1) / theta_steps

            pdf = mass / ((math.cos(theta1) - math.cos(theta2)) * (phi2 - phi1))

            print(phi1, phi2)
            print(theta1, theta2)
            print(math.sin((theta2 + theta1) / 2))
            print(math.sin((math.pi / 2) * (theta_step + 0.5) / theta_steps))
            print(pdf)
            print("==========")

            estimator += pdf * math.sin((theta2 + theta1) / 2) * d_phi * d_theta
            # estimator += math.sin((theta2 + theta1) / 2) * d_phi * d_theta

    print(estimator)

if __name__ == "__main__":
    run()
