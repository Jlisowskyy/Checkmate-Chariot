#!/usr/bin/env python
import cma,math,random,numpy

def strength(x):
    if x < -1.0:
        x = -1.0
    if x > +1.0:
        x = +1.0
    xx = x * 2.0 + 1.0
    return 2 * math.log(2.0 * xx + 2.1) - 2.0 * xx - 1.0;

def weakness(x):
    return -strength(x)

def logistic(x):
    return 1 / (1 + math.exp(-x))

def outcome(x):
    if not numpy.isscalar(x):
        x = x[0]
    if random.random() < logistic(strength(x)):
        return 0
    else:
        return 1

#cma.fmin(weakness, [0], 1)

samples = 10000
replications = 100
x_star = -0.525
regret = 0

for i in range(replications):
    while True:
        res = cma.fmin(outcome,
                      [0],
                       0.1,
                       tolfun = 0,
                       tolfunhist = 0,
                       maxfevals = samples,
                       noise_handling = [10, samples])
        x_tilde = res[5][0]
        if x_tilde > -1 and x_tilde < 1:
            break
    regret += logistic(strength(x_star)) - logistic(strength(x_tilde))
    print "i =", i, "; average regret:", regret / (i + 1)
