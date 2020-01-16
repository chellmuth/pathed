from interpolation import Grid

def test_basic():
    x, y, z = 10, 10, 10
    data = [
        1 for _ in range(x * y * z)
    ]
    grid = Grid(x, y, z, data)

    assert grid.interpolate(3.4, 2.0, 5.555) == 1

def test_custom_grid():
    x, y, z = 10, 10, 10
    data = [
        i for i in range(x * y * z)
    ]
    grid = Grid(x, y, z, data)

    # index  x  y  z  value
    #   000  3  2  5    523
    #   001  3  2  6    623
    #   010  3  3  5    533
    #   011  3  3  6    633
    #   100  4  2  5    524
    #   101  4  2  6    624
    #   110  4  3  5    534
    #   111  4  3  6    634

    assert grid.lookup(3, 2, 5) == 523
    assert grid.lookup(3, 2, 6) == 623
    assert grid.lookup(3, 3, 5) == 533

    # xd = 0.5
    # yd = 0.1
    # zd = 0.75

    # c_00 = 523 * 0.5 + 524 * 0.5 = 523.5
    # c_01 = 623 * 0.5 + 623 * 0.5 = 623.5
    # c_10 = 533 * 0.5 + 534 * 0.5 = 533.5
    # c_11 = 633 * 0.5 + 634 * 0.5 = 633.5

    # c__0 = 523.5 * 0.9 + 533.5 * 0.1 = 524.5
    # c__1 = 623.5 * 0.9 + 633.5 * 0.1 = 624.5

    # c___ = 524.5 * 0.25 + 624.5 * 0.75 = 599.5

    assert grid.interpolate(3.5, 2.1, 5.75) == 599.5

# def test_bounds():
#     x, y, z = 10, 10, 10
#     data = [
#         i for i in range(x * y * z)
#     ]
#     grid = Grid(x, y, z, data)

#     assert grid.interpolate(0.5, 0.0, 0.0) == 1
#     assert grid.interpolate(0.0, 0.0, 0.0) == 0
#     assert grid.interpolate(10.0, 10.0, 10.0) == 0
#     assert grid.interpolate(9.999, 9.999, 9.999) == 0

def test_midpoint_rule():
    x, y, z = 10, 10, 10
    data = [
        i for i in range(x * y * z)
    ]
    grid = Grid(x, y, z, data)

    p0 = (8.4, 3.6, 7.2)
    p1 = (3.5, 1.0, 3.1)

    c0 = grid.interpolate(*p0)
    c1 = grid.interpolate(*p1)

    mean = (c0 + c1) * 0.5

    midpoint = (
        (p1[0] + p0[0]) * 0.5,
        (p1[1] + p0[1]) * 0.5,
        (p1[2] + p0[2]) * 0.5,
    )

    c_mid = grid.interpolate(*midpoint)

    assert c_mid == mean
