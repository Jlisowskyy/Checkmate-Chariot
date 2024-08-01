#!/usr/bin/env python
#############################################################################
"""
 htest.py

 Remi Coulom

 August, 2011
"""
#############################################################################
import sys
sys.path.append('../../swig')
from clop import *

class MyExperimentBuilder(ExperimentBuilder):
    def build(self):
        #####################################################################
        "Build experiment"
        #####################################################################
        p = self.problem_class()

        exp = Experiment(p)
        pf = CPFQuadratic(p.GetDimensions())
        #pf = CLinearCubic(p.GetDimensions())
        pf.SetPriorStrength(1e-2)
        exp.reg = CRegression(exp.res, pf)
        exp.reg.SetRefreshRate(0.1)
        exp.reg.SetLocalizationHeight(self.LocalizationHeight)
        #exp.reg.SetLocalizationPower(self.LocalizationPower)
        exp.me = CMESampleMean(exp.reg)
        #exp.me = CMERegressionMAPMax(exp.reg)
        exp.sp = CSPWeight(exp.reg, 100)
    
        return exp

def test_class(problem_class):
    #########################################################################
    "test a class"
    #########################################################################
    eb = MyExperimentBuilder()
    eb.problem_class = problem_class
    
    for H in [1, 2, 3, 4, 6, 8, 10, 12]:
        eb.LocalizationHeight = H
        s = problem_class.__name__ + "-H=%02d" % int(eb.LocalizationHeight)
        eb.threads(repeats = 1000, samples = 10000000, filename = s)

#############################################################################
"Main program"
#############################################################################
#for p in [CPLog1D, CPFlat]:
for p in [CPNonQuadraticND, CPComplicated1D, CPLog1D, CPFlat, CPNonQuadratic2D, CPRosenbrock, CPVolcano]:
    test_class(p)
