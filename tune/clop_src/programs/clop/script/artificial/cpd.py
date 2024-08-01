#!/usr/bin/python
# -*- coding: utf-8 -*-
#############################################################################
"""
 Shortcuts to create experiment builders and collect check-point data
"""
#############################################################################
import sys
sys.path.append('../../swig')
from clop import *

class UCTExperimentBuilder(ExperimentBuilder):
    def build(self):
        exp = Experiment(problem())
        bast = CBAST(exp.res)
        exp.sp = bast
        exp.me = bast
        return exp

class QLRExperimentBuilder(ExperimentBuilder):
    def build(self):
        exp = Experiment(problem())
        pf = CPFQuadratic(exp.p.GetDimensions())
        #pf = CIndependentQuadratic(exp.p.GetDimensions())
        pf.SetPriorStrength(1e-2)
        exp.reg = CRegression(exp.res, pf)
        exp.me = CMEQLRMAP(exp.reg)
        exp.reg.SetRefreshRate(0.1)
        exp.reg.SetLocalizationHeight(self.LH)
        exp.reg.SetLocalizationPower(self.LP)
        #var = CDFVarianceSamples(exp.reg, 1)
        #var.SetMinSamples(1)
        #exp.sp = CSPVOptimal(exp.reg, var)
        exp.sp = CSPWeight(exp.reg, 25)
        return exp

class CEMExperimentBuilder(ExperimentBuilder):
    def build(self):
        exp = Experiment(problem())
        cem = CCrossEntropy(exp.res)
        exp.sp = cem
        exp.me = cem
        return exp

def name(eb):
    exp = eb.build()
    n = exp.p.__class__.__name__ + '-' + exp.me.__class__.__name__ 
    if eb.__class__ == QLRExperimentBuilder:
        n += '-' + str(eb.LH) + '-' + str(eb.LP)
    return n

def run_uct(repeats, samples):
    eb = UCTExperimentBuilder()
    eb.threads(repeats, samples, name(eb))
    print

def run_cem(repeats, samples):
    eb = CEMExperimentBuilder()
    eb.threads(repeats, samples, name(eb))
    print

def run_clop(repeats, samples, LH = 0.0, LP = 1.0):
    eb = QLRExperimentBuilder()
    eb.LH = LH
    eb.LP = LP
    print "LH = %f" % eb.LH
    print "LP = %f" % eb.LP
    exp = eb.build()
    eb.threads(repeats, samples, name(eb))
    print

def set_problem(p):
    global problem
    problem = p
