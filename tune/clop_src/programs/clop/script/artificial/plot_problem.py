#!/usr/bin/env python
#############################################################################
"""
 plot_problem.py

 plot an artificial problem

 usage: ./plot_problem.py <class_name>

 examples:
 ./plot_problem.py CPRosenbrock
 ./plot_problem.py CPQuadratic2D
 ./plot_problem.py CPNonQuadratic2D

"""
#############################################################################
import sys
sys.path.append('../../swig')
from clop import *

if len(sys.argv) < 2:
    print __doc__
    sys.exit()

class_name = sys.argv[1]
print "class_name =", class_name
p = eval(class_name)()

dimensions = p.GetDimensions();
print "p.GetDimensions() =", p.GetDimensions()

v = doubleArray(dimensions)
p.GetOptimalParameters(v)
print "p.GetOptimalParameters():",
for i in range(dimensions):
    print v[i],
print

if dimensions == 1:
    print "no plot in 1D"
elif dimensions == 2:
    resolution = 50
    file = open('plot_problem.dat', 'w')
    for x in range(resolution + 1):
        v[0] = -1.0 + 2.0 * float(x) / (resolution + 1)
        for y in range(resolution + 1):
            v[1] = -1.0 + 2.0 * float(y) / (resolution + 1)
            file.write(str(v[0]) + ' ' + str(v[1]) + ' ' + str(p.GetProba(v)) + '\n')
        file.write('\n');
    file.close()
    gp = Gnuplot()
    gp.send('load "plot_problem.gpi"')
    raw_input()
