#!/usr/bin/env python
from distutils.core import setup
from distutils.extension import Extension

def read_list(s):
    return open(s + '.tmp').readline().split()

def do_setup(s):
    module = Extension('_' + s,
                       sources = [s + '.i'],
                       swig_opts = read_list('swig_opts'),
                       extra_objects = read_list('extra_objects'),
                       extra_compile_args = read_list('extra_compile_args'),
                       extra_link_args = read_list('extra_link_args'))
    
    setup(name=s, ext_modules=[module], py_modules=[s])
