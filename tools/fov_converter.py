import math

def fov_x_deg_to_fov_y_deg(fov_x, width, height):
    fov_x_rad = fov_x / 180 * math.pi
    fov_y_rad = 2. * math.atan(math.tan(fov_x_rad / 2.) * height / width)
    fov_y_deg = fov_y_rad / math.pi * 180

    return fov_y_deg
