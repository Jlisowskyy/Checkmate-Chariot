#!/usr/bin/env python
from cpd import *

def problem():
#    return CPQuadratic1D(-0.25, 2.0, 4*(7*7))
    return CPNonQuadraticND(5)
#    return CPQuadraticND(4)
#    return CPLog1D()
#    return CPQuadratic1D(-0.2, 0.5, 4.0)
#    return CPRosenbrock()
#    return CPComplicated1D()
#    return CPNonQuadratic2D()
#    return CPSinExp()

set_problem(problem)
run_cem(100, 100000)
run_uct(100, 100000)
run_clop(100, 100000, LH = 3.0, LP = 1.0)
run_clop(100, 100000, LH = 4.0, LP = 1.0)
run_clop(100, 100000, LH = 5.0, LP = 1.0)
