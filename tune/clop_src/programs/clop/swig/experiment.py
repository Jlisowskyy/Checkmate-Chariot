# -*- coding: utf-8 -*-
#############################################################################
"""
 
 experiment.py
 
 Rémi Coulom
 
 April, 2009
 
 This file augments the SWIG interface with python-specific tools
 
"""
#############################################################################
from clop_swig import CResults
from clop_swig import COutcome
from clop_swig import CRegression
from clop_swig import CPFQuadratic
from clop_swig import CMERegressionMAPMax
from clop_swig import Random
from clop_swig import CQLRIO
from clop_swig import CArtificialExperiment
from clop_swig import vectord
from clop_swig import doubleArray
from gnuplot import Gnuplot


class Experiment:
    #########################################################################
    "contains all data for an experiment: results, sampling policy, etc."
    #########################################################################

    def __init__(self, p):
        #####################################################################
        "constructor"
        #####################################################################
        self.p = p
        self.res = CResults(p.GetDimensions())

    def seed(self, n):
        #####################################################################
        "seed random elements of the experiment"
        #####################################################################
        self.sp.Seed(n)
        self.p.Seed(n + 1)

    def add_clop(self):
        #####################################################################
        "add reg (and me) to this experiment"
        #####################################################################
        self.reg = CRegression(self.res,
                               CPFQuadratic(self.p.GetDimensions()))
        self.me = CMERegressionMAPMax(self.reg)

    def reset(self):
        #####################################################################
        "reset"
        #####################################################################
        self.res.Reset()

    def max(self):
        #####################################################################
        "convert vectord to a list"
        #####################################################################
        v = vectord(self.p.GetDimensions())
        if self.me.MaxParameter(v):
            result = []
            for i in range(v.size()):
                result.append(v[i])
            return result
        else:
            return None

    def run(self, n, trace = False):
        #####################################################################
        "collect n samples"
        #####################################################################
        for i in range(n):
            v = self.sp.NextSample(self.res.GetSamples())
            r = self.p.GetOutcome(v)
            if r == COutcome.Unknown:
                return
            self.res.AddSample(v, r)

            if trace:
                print "%6d" % i, r, v, self.max()
        self.res.Refresh();

    def gnuplot(self, reg):
        #####################################################################
        "plot with gnuplot"
        #####################################################################
        if not self.__dict__.has_key('gp'):
            self.gp = Gnuplot()

        CQLRIO.DumpSamples('samples.dat', self.reg)

        if self.p.GetDimensions() == 1:
            CQLRIO.Distrib(reg)
            self.plot_to_file('distrib.dat', 100)
            self.gp.send('load "clop.gpi"')
        elif self.p.GetDimensions() == 2:
            CQLRIO.Distrib(reg)
            self.plot_to_file_2d('distrib.dat', 40)
            self.gp.send('load "samples2d.gpi"')

    def new_gnuplot(self, reg):
        #####################################################################
        "create a new gnuplot window"
        #####################################################################
        self.gp = Gnuplot()
        self.gnuplot(reg)

    def plot_to_file(self, filename, resolution):
        #####################################################################
        "write 1D model data to a file"
        #####################################################################
        f = open(filename, 'w')
        for i in range(resolution + 1):
            x = doubleArray(1)
            x[0] = -1.0 + (2.0 * i) / resolution
            f.write("%f %f %f\n" % (x[0],
                                    self.p.GetProba(x),
                                    self.p.GetStrength(x)))

    def plot_to_file_2d(self, filename, resolution):
        #####################################################################
        "write 2D model data to a file"
        #####################################################################
        f = open(filename, 'w')
        for i in range(resolution + 1):
            x = -1.0 + (2.0 * i) / resolution
            for j in range(resolution + 1):
                y = -1.0 + (2.0 * j) / resolution
                v = doubleArray(2)
                v[0] = x
                v[1] = y
                f.write("%f %f %f %f\n" % (x,
                                           y,
                                           self.p.GetProba(v),
                                           self.p.GetStrength(v)))
            f.write('\n')

    def experiment_setup(self):
        #####################################################################
        "get a CArtificialExperiment for CRepeatThreads"
        #####################################################################
        return CArtificialExperiment(self.p, self.sp, self.me, self.res)
