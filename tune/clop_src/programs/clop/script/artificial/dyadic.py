#!/usr/bin/env python
import sys
sys.path.append('../../swig')
from clop import *

n = 4
if len(sys.argv) >= 2:
    n = int(sys.argv[1])

print "n =", n
print

sp = CSPDyadic(n)
for i in range(1 << n):
    s = sp.NextSample(i)
    for j in range(n):
        print int(s[j] + 1),
    print
