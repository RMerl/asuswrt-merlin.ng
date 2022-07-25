#!/usr/bin/env python2
# SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)

"""
setup.py file for SWIG libfdt
Copyright (C) 2017 Google, Inc.
Written by Simon Glass <sjg@chromium.org>
"""

from distutils.core import setup, Extension
import os
import re
import sys


VERSION_PATTERN = '^#define DTC_VERSION "DTC ([^"]*)"$'


def get_version():
    version_file = "../version_gen.h"
    f = open(version_file, 'rt')
    m = re.match(VERSION_PATTERN, f.readline())
    return m.group(1)


setupdir = os.path.dirname(os.path.abspath(sys.argv[0]))
os.chdir(setupdir)

libfdt_module = Extension(
    '_libfdt',
    sources=['libfdt.i'],
    include_dirs=['../libfdt'],
    libraries=['fdt'],
    library_dirs=['../libfdt'],
    swig_opts=['-I../libfdt'],
)

setup(
    name='libfdt',
    version=get_version(),
    author='Simon Glass <sjg@chromium.org>',
    description='Python binding for libfdt',
    ext_modules=[libfdt_module],
    py_modules=['libfdt'],
)
