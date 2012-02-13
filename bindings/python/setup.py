import sys
from distutils.core import setup
from Cython.Distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = []
ext_modules.append(Extension('jsonparser', 
    ['./jsonparser.pyx'], language = 'c++'))

setup(
    name = 'json-parser python wrapper',
    cmdclass = {'build_ext' : build_ext},
    ext_modules = ext_modules
)