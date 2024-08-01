#!/usr/bin/python
# -*- coding: utf-8 -*-
#############################################################################
"""
 cubic.py

 Rémi Coulom

 February, 2010
"""
#############################################################################
import sys
sys.path.append('../../swig')
from clop import *

#
# Build experiment
#
exp = Experiment(CPLog1D())
exp.reg = CRegression(exp.res, CLinearCubic(exp.p.GetDimensions()))
exp.me = CMEQLRMAP(exp.reg)
exp.reg.SetRefreshRate(0.1)
exp.reg.SetLocalizationHeight(2.0)

#
# Plot experiment
#
reg = exp.reg
var = CDFVarianceSamples(reg, 1)
var.SetMinSamples(exp.p.GetDimensions())
exp.sp = CSPVOptimal(exp.reg, var, 0)
#exp.sp = CSPUniform(exp.p.GetDimensions())

exp.reset()
exp.seed(5)
exp.run(2000)
exp.reg.SetUniformWeights()
exp.new_gnuplot(reg)
