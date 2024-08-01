#!/usr/bin/env python
import sys
sys.path.append('../../swig')
from clop import *

class MyExperimentBuilder(ExperimentBuilder):
    def build(self):
        exp = Experiment(COlivier(1, 0.1, 1)) # Dimensions, Power, PMax
        pf = CPFQuadratic(exp.p.GetDimensions())
        pf.SetPriorStrength(1e-1)
        exp.reg = CRegression(exp.res, pf)
        exp.reg.SetRefreshRate(0.1)
        exp.me = CMEQLRMAP(exp.reg)
        var = CDFVarianceSamples(exp.reg, 10)
        var.SetMinSamples(exp.p.GetDimensions())
        exp.sp = CSPVOptimal(exp.reg, var)
    
        return exp

#############################################################################
"Main program"
#############################################################################
eb = MyExperimentBuilder()
eb.gnuplot(samples = 1000, seed = 437)
#eb.threads(repeats = 10, samples = 100000, use_elo = False)
