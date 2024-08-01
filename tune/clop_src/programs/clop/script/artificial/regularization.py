#!/usr/bin/env python
import sys
sys.path.append('../../swig')
from clop import *

class MyEB(ExperimentBuilder):
    def build(self):
        p = CPMultiple(5, CPRosenbrock())
        exp = Experiment(p)
        pf = CPFQuadratic(p.GetDimensions())
        pf.SetPriorStrength(self.Prior)
        exp.reg = CRegression(exp.res, pf)
        exp.reg.SetRefreshRate(0.1)
        exp.reg.SetLocalizationHeight(3.0)
        exp.reg.SetLocalizationPower(0.0)
        exp.me = CMESampleMean(exp.reg)
        exp.sp = CSPWeight(exp.reg, 100, 100)
        exp.reg.SetMaxWeightIterations(self.MWI)
        
        return exp

if False:
    for i in range(8):
        eb = MyEB()
        eb.MWI = 0
        eb.Prior = pow(10.0, i - 6)
        eb.threads(100, 10000, "Prior=" + str(eb.Prior))

if True:
    for i in range(0, 12):
        eb = MyEB()
        eb.MWI = i
        eb.Prior = 1e-2
        eb.threads(100, 100000, "MWI=" + str(eb.MWI))
