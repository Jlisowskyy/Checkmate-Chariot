# -*- coding: utf-8 -*-
#############################################################################
"""

 gnuplot.py

 Rémi Coulom

 April, 2009

"""
#############################################################################
from subprocess import Popen, PIPE
import time


class Gnuplot:
    #########################################################################
    "bidirectional pipe with gnuplot"
    #########################################################################

    def __init__(self):
        #####################################################################
        "constructor"
        #####################################################################
        self.gp = Popen(['gnuplot', '-persist', '-background', 'white'], stdin = PIPE, stderr = PIPE)

    def sync(self):
        #####################################################################
        "wait for gnuplot to be ready"
        #####################################################################
        self.gp.stdin.write('print "sync"\n')
        self.gp.stdin.flush();
        while True:
            line = self.gp.stderr.readline()
            if line == 'sync\n':
                break
            else:
                print "gnuplot error: ", line

    def send(self, s):
        #####################################################################
        "send s to gnuplot"
        #####################################################################
        self.gp.stdin.write(s + '\n')
        self.gp.stdin.flush()
        self.sync()
