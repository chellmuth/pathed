import pprint
from sympy import Matrix, MatrixSymbol, symbols

def run():
    CX, CY, CZ, SX, SY, SZ = symbols("c0, c1, c2, s0, s1, s2")

    RotX = Matrix([
        [ 1, 0, 0, 0, ],
        [ 0, CX, -SX, 0, ],
        [ 0, SX, CX, 0, ],
        [ 0, 0, 0, 1, ],
    ])
    RotY = Matrix([
        [ CY, 0, SY, 0, ],
        [ 0, 1, 0, 0, ],
        [ -SY, 0, CY, 0, ],
        [ 0, 0, 0, 1, ],
    ])
    RotZ = Matrix([
        [ CZ, -SZ, 0, 0, ],
        [ SZ, CZ, 0, 0, ],
        [ 0, 0, 1, 0, ],
        [ 0, 0, 0, 1, ],
    ])

    pprint.pprint(RotX)
    pprint.pprint(RotY)
    pprint.pprint(RotZ)

    # print("XYZ")
    # pprint.pprint(RotZ * RotY * RotX)

    # print("YXZ")
    # pprint.pprint(RotZ * RotX * RotY)

    print("Guessing")
    pprint.pprint(RotY * RotX * RotZ)

if __name__ == "__main__":
    run()
