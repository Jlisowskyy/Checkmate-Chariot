#!/usr/bin/env python
# -*- coding: utf-8 -*-
#############################################################################
"""
 test_swig.py

 Rémi Coulom

 April, 2009
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
        #
        # Choose a problem
        #
        #p = CPSinExp()
        #p = CPComplicated1D()
        #p = CPLog1D()
        #p = CPFish1D(0.1)
        #p = CPPositive()
        #p = CPQuadratic1D(-0.25, 2.0, 4*(7*7))
        #p = CPQuadratic1D(-0.2, 0.5, 4.0)
        #p = CPPower1D()

        #params = doubleArray(4)
        #params[0] = -0.2
        #params[1] = 0.5
        #params[2] = -5.0
        #params[3] = 3.0
        #p = CPParametric(CPFCubic(1), params)
        #p.params = params

        #p = CPNonQuadratic2D()
        #p = CPQuadratic2D()
        #p = CPDonut(2)
        #p = CPRosenbrock()
        #p = CPIllIndependent()
        #p = CPIllCorrelated()

        #p = CPNonQuadraticND(5)
        #p = CPVolcano(2, 0.3)
        p = CPMultiple(5, CPIllCorrelated())
        #p = CPQuadraticND(2)
        #p = CPFlat(1)

        #
        # Build experiment
        #
        exp = Experiment(p)
        
        #
        # Parametric regression
        #
        if True:
            #pf = CPFIndependentQuadratic(p.GetDimensions())
            pf = CPFQuadratic(p.GetDimensions())
            #pf = CPFCubic(p.GetDimensions())
            pf.SetPriorStrength(1e-2)
            exp.reg = CRegression(exp.res, pf)
            exp.reg.SetRefreshRate(0.1)
            exp.reg.SetLocalizationHeight(3.0)
            #exp.reg.SetLocalizationPower(1.0 / 4.0)
            exp.me = CMESampleMean(exp.reg)
            #exp.me = CMERegressionMAPMax(exp.reg)
            exp.sp = CSPWeight(exp.reg, 1000, 100)
        #
        # Alternative methods
        #
        else:
            alt = CCrossEntropy(exp.res, 0.9)
            #alt = CBAST(exp.res)
            #alt = CSPSA(exp.res, 0.05, 0.0, 0.0000001, 0.602, 0.101)

            #alt = CRSPSA(exp.res, 1000,  # Batch size
            #                      1.0,   # Batch growth
            #                      1.00,  # Eta+
            #                      0.90,  # Eta-
            #                      0.0,   # DeltaMin
            #                      0.02,  # DeltaMax
            #                      0.019, # Delta0
            #                      25.0   # Rho
            #            )

            exp.sp = alt
            exp.me = alt
    
        return exp

#############################################################################
"Main program"
#############################################################################
eb = MyExperimentBuilder()
eb.threads(repeats = 100, samples = 100000)
#eb.gnuplot(samples = 500, seed = 95)
#eb.multi_gnuplot(samples = 500)
#eb.tikz(samples = 10, seed = 368)

if False:
    exp = eb.build()
    exp.seed(79)
    for i in range(50):
        exp.run(100)
        exp.gnuplot(exp.reg)
        sleep(1)
        print exp.reg.GetSamples()
