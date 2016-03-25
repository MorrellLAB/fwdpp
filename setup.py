#Installation script for fwdpp's Python module

from __future__ import print_function
from distutils.core import setup, Extension
import platform, glob, sys, subprocess, os

GLOBAL_COMPILE_ARGS=['-std=c++11']
LINK_ARGS=["-std=c++11"]
LIBS=["gsl","gslcblas"]

extensions = [
    Extension("fwdpp_python.fwdpp_python",
              sources=["fwdpp_python/fwdpp_python.cc"], # the Cython source and additional C++ source files
              language="c++",                        # generate and compile C++ code
              include_dirs=['.','include','..'], 
              extra_compile_args=GLOBAL_COMPILE_ARGS,
              extra_link_args=LINK_ARGS,
              libraries=LIBS),
    ]

PKGS=['fwdpp_python']

long_desc="Expost fwdpp's basic types to Python"

setup(name='fwdpp_python',
      version='0.4.7',
      author='Kevin R. Thornton',
      author_email='krthornt@uci.edu',
      maintainer='Kevin R. Thornton',
      maintainer_email='krthornt@uci.edu',
      url='http://www.molpopgen.org',
      description="Expose fwdpp's basic types to Python",
      long_description=long_desc,
      download_url='http://github.com/molpopgen/fwdpp',
      classifiers=['Intended Audience :: Science/Research',
                   'Topic :: Scientific/Engineering :: Bio-Informatics',
                   'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)'],
      platforms=['Linux','OS X'],
      license='GPL >= 2',
      provides=['fwdpp_python'],
      obsoletes=['none'],
      packages=PKGS,
      py_modules=[],
      scripts=[],
#      data_files=[(doc_dir, ['COPYING', 'README.rst'])],
      ##Note: when installing the git repo, headers will be put in /usr/local/include/pythonVERSION/fwdpy
      headers=glob.glob("fwdpp_python/include/*.hpp"),
      ext_modules=extensions,
     )


