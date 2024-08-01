#!/usr/bin/python
# -*- coding: utf-8 -*-
#############################################################################
"""
 weights.py

 Rémi Coulom

 January, 2010
"""
#############################################################################
import sys
sys.path.append('../../swig')
from clop import *

exp = Experiment(CPDiscontinuous())
exp.add_clop()
exp.reg.SetRefreshRate(0.1)

var = CDFVarianceSamples(exp.reg, 10)
var.SetMinSamples(exp.p.GetDimensions())
exp.sp = CSPVOptimal(exp.reg, var, 0)

exp.seed(2)
exp.reg.SetLocalizationHeight(2.0)
exp.run(2000)

#exp.reg.SetUniformWeights()
exp.gnuplot(exp.reg)
