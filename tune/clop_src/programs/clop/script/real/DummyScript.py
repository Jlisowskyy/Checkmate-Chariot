#!/usr/bin/env python
#############################################################################
"""
 DummyScript.py

 This is an example script for use with parallel optimization. In order to
 apply clop algorithms to your own problem, you should write a script that
 behaves like this one.

 Arguments are:
  #1: processor id (symbolic name, typically a machine name to ssh to)
  #2: seed (integer)
  #3: parameter id of first parameter (symbolic name)
  #4: value of first parameter (float)
  #5: parameter id of second parameter (optional)
  #6: value of second parameter (optional)
  ...

 This script should write the game outcome to its output:
  W = win
  L = loss
  D = draw

 For instance:
  $ ./DummyScript.py node-01 4 param 0.2
  W
"""
#############################################################################
import sys
import math
import random
import time

#
# Log script invocations
#
f = open('DummyScript.log', 'a')
print >>f, sys.argv

#
# Print doc if not enough parameters
#
if len(sys.argv) < 5:
    print __doc__
    sys.exit()

#
# Sleep for a random amount of time
#
random.seed(int(sys.argv[2]))
time.sleep(random.random() * 2)

#
# Parse parameter values
#
i = 4
params = []
while i < len(sys.argv):
    params.append(float(sys.argv[i]))
    i += 2

#
# Compute winning probability
#
d2 = 0
for i in range(len(params)):
    delta = params[i] - 0.3456789
    d2 += delta * delta * 10

draw_elo = 100
draw_rating = draw_elo * math.log(10.0) / 400.0
win_p = 1.0 / (1.0 + math.exp(d2 + draw_rating))
loss_p = 1.0 / (1.0 + math.exp(-d2 + draw_rating))

#
# Draw a random game according to this probability
#
r = random.random()
if r < loss_p:
    print "L"
elif r > 1.0 - win_p:
    print "W"
else:
    print "D"
