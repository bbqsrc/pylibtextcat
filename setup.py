#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Python 3 Bindings for libtextcat
#
# Copyright (c) 2008-2010 Per Øyvind Karlsen <peroyvind@mandriva.org>
# Copyright (c) 2011 Brendan Molloy <brendan@bbqsrc.net>
#
# All rights reserved.
import sys, os
from warnings import warn
from setuptools import setup, Extension
from distutils.ccompiler import get_default_compiler

descr = "Python bindings for libTextCat"
long_descr = """PylibTextCat provides a python interface for the libTextCat
library."""
version = '0.2'
version_define = [('VERSION', '"%s"' % version)]

# FIXME: Probably some nicer way to do this
if 'sdist' in sys.argv:
    os.system('git log . > ChangeLog')
c_files = ['libtextcat.c']
compile_args = []
if get_default_compiler() in ('cygwin', 'emx', 'mingw32', 'unix'):
    warnflags = ['-Wall', '-Wextra']
    compile_args.extend(warnflags)

library = ['textcat']
extens=[Extension('textcat', c_files, extra_compile_args=compile_args, libraries=library, define_macros=version_define)]

setup(
    name = "pylibtextcat",
    version = version,
    description = descr,
    author = "Per Øyvind Karlsen",
    author_email = "peroyvind@mandriva.org",
    url = "http://launchpad.net/pylibtextcat",
    license = 'BSD',
    long_description = long_descr,
    platforms = sys.platform,
    classifiers = [
        'Development Status :: 4 - Beta',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Operating System :: POSIX :: Linux'
    ],
    ext_modules = extens,
    test_suite = 'tests',    
)
