# -*- coding: utf-8 -*-
#############################################################################
"""
 experiment_builder.py

 Rémi Coulom

 April, 2009
"""
#############################################################################
from clop_swig import CCPLConsole,CRepeatThreads,hardware_concurrency,CPFQuadratic,CRegression
from gnuplot import Gnuplot
from time import sleep
from math import sqrt,log

default_threads = 4

def set_default_threads(n):
    #########################################################################
    "Set default number of threads"
    #########################################################################
    global default_threads
    default_threads = n

class ExperimentBuilder:
    #########################################################################
    "A virtual function to build an experiment, and useful interfaces"
    #########################################################################

    def build(self):
        #####################################################################
        "Build experiment. This function should be overriden by descendants."
        #####################################################################
        exp = Experiment()
        return exp

    def gnuplot(self, samples, seed):
        #####################################################################
        "plot experiment result"
        #####################################################################
        exp = self.build()
        exp.seed(seed)
        exp.run(samples)
        if exp.__dict__.has_key('reg'):
            exp.gnuplot(exp.reg)
        else:
            pf = CPFQuadratic(exp.p.GetDimensions())
            exp.reg = CRegression(exp.res, pf)
            exp.gnuplot(exp.reg)

    def multi_gnuplot(self, samples):
        #####################################################################
        "infinite loop of random runs"
        #####################################################################
        exp = self.build()
        seed = 0
        while True:
            print "seed =", seed
            exp.res.Reset()
            exp.seed(seed)
            exp.run(samples)
            exp.gnuplot(exp.reg)
            sleep(1.0)
            seed = seed + 1
    
    def threads(self,
                repeats,
                samples,
                filename = "MeanError",
                threads = hardware_concurrency()):
        #####################################################################
        "repeat threads"
        #####################################################################

        if default_threads < 0:
            threads = -default_threads

        if threads <= 0:
            threads = default_threads

        cplc = CCPLConsole(1)
        rts = CRepeatThreads(repeats, samples, cplc)

        rate = 1/sqrt(sqrt(sqrt(10)))
        n = samples
        while True:
            n = n * rate
            if n < 10:
                break;
            rts.AddCheckPoint(int(n)) 
        
        exp_list = []
        es_list = []
    
        for i in range(threads):
            exp = self.build()
            es = exp.experiment_setup()
            rts.AddThread(es)
            exp_list.append(exp)
            es_list.append(es)
    
        print "sp =", exp_list[0].sp.__class__.__name__
        print "me =", exp_list[0].me.__class__.__name__
        print "problem =", exp_list[0].p.__class__.__name__
        print "dimensions =", exp_list[0].p.GetDimensions()
        print "repeats =", repeats
        print "samples =", samples
        print "threads =", threads
        print "filename =", filename
    
        rts.Start()
        rts.WaitForTermination()

        #
        # Write file with check-point data
        #
        filename = 'cpd-' + filename + '.dat'
        f = open(filename, 'w')
        for i in range(rts.GetCheckPoints()):
            cp = rts.GetCheckPointData(i)
            print >> f, "%3d" % i,\
                        "%7d" % cp.GetRepeats(),\
                        "%8d" % cp.GetSamples(),\
                        "%13f" % cp.GetMeanError(),
            for j in range(11):
                print >> f, "%13f" % cp.GetPercentile(j * 0.1),
            print >> f
        f.close()
    
    def tikz(self, samples, seed):
        #####################################################################
        "tikz output"
        #####################################################################
        exp = self.build()
        exp.seed(seed)
        exp.run(samples)
        exp.tikz()
