#!/usr/bin/env python3
# SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)

"""
setup.py file for SWIG libfdt
Copyright (C) 2017 Google, Inc.
Written by Simon Glass <sjg@chromium.org>

This script is modified from the upstream version, to fit in with the U-Boot
build system.

Files to be built into the extension are provided in SOURCES
C flags to use are provided in CPPFLAGS
Object file directory is provided in OBJDIR
Version is provided in VERSION

If these variables are not given they are parsed from the Makefiles. This
allows this script to be run stand-alone, e.g.:

    ./pylibfdt/setup.py install [--prefix=...]
"""

from setuptools import setup, Extension
from setuptools.command.build_py import build_py as _build_py
import os
import re
import sys

try:
    from setuptools import sic
except ImportError:
    pass

srcdir = os.path.dirname(__file__)

with open(os.path.join(srcdir, "../README"), "r") as fh:
    long_description = fh.read()

# Decodes a Makefile assignment line into key and value (and plus for +=)
RE_KEY_VALUE = re.compile(r'(?P<key>\w+) *(?P<plus>[+])?= *(?P<value>.*)$')

def get_top_builddir():
    if '--top-builddir' in sys.argv:
        index = sys.argv.index('--top-builddir')
        sys.argv.pop(index)
        return sys.argv.pop(index)
    else:
        return os.path.join(srcdir, '..')

top_builddir = get_top_builddir()

def ParseMakefile(fname):
    """Parse a Makefile to obtain its variables.

    This collects variable assigments of the form:

        VAR = value
        VAR += more

    It does not pick out := assignments, as these are not needed here. It does
    handle line continuation.

    Returns a dict:
        key: Variable name (e.g. 'VAR')
        value: Variable value (e.g. 'value more')
    """
    makevars = {}
    with open(fname) as fd:
        prev_text = ''  # Continuation text from previous line(s)
        for line in fd.read().splitlines():
          if line and line[-1] == '\\':  # Deal with line continuation
            prev_text += line[:-1]
            continue
          elif prev_text:
            line = prev_text + line
            prev_text = ''  # Continuation is now used up
          m = RE_KEY_VALUE.match(line)
          if m:
            value = m.group('value') or ''
            key = m.group('key')

            # Appending to a variable inserts a space beforehand
            if 'plus' in m.groupdict() and key in makevars:
              makevars[key] += ' ' + value
            else:
              makevars[key] = value
    return makevars

def GetEnvFromMakefiles():
    """Scan the Makefiles to obtain the settings we need.

    This assumes that this script is being run from the top-level directory,
    not the pylibfdt directory.

    Returns:
        Tuple with:
            List of swig options
            Version string
            List of files to build
            List of extra C preprocessor flags needed
            Object directory to use (always '')
    """
    basedir = os.path.dirname(os.path.dirname(os.path.abspath(sys.argv[0])))
    swig_opts = ['-I%s' % basedir]
    makevars = ParseMakefile(os.path.join(basedir, 'Makefile'))
    version = '%s.%s.%s' % (makevars['VERSION'], makevars['PATCHLEVEL'],
                            makevars['SUBLEVEL'])
    makevars = ParseMakefile(os.path.join(basedir, 'libfdt', 'Makefile.libfdt'))
    files = makevars['LIBFDT_SRCS'].split()
    files = [os.path.join(basedir, 'libfdt', fname) for fname in files]
    files.append('libfdt.i')
    cflags = ['-I%s' % basedir, '-I%s/libfdt' % basedir]
    objdir = ''
    return swig_opts, version, files, cflags, objdir


progname = sys.argv[0]
files = os.environ.get('SOURCES', '').split()
cflags = os.environ.get('CPPFLAGS', '').split()
objdir = os.environ.get('OBJDIR')
try:
    version = sic(os.environ.get('VERSION'))
except:
    version = os.environ.get('VERSION')
swig_opts = os.environ.get('SWIG_OPTS', '').split()

# If we were called directly rather than through our Makefile (which is often
# the case with Python module installation), read the settings from the
# Makefile.
if not all((swig_opts, version, files, cflags, objdir)):
    swig_opts, version, files, cflags, objdir = GetEnvFromMakefiles()

libfdt_module = Extension(
    '_libfdt',
    sources=files,
    include_dirs=[os.path.join(srcdir, 'libfdt')],
    library_dirs=[os.path.join(top_builddir, 'libfdt')],
    swig_opts=swig_opts,
)

class build_py(_build_py):
    def run(self):
        self.run_command("build_ext")
        return super().run()

setup(
    name='libfdt',
    version=version,
    cmdclass = {'build_py' : build_py},
    author='Simon Glass',
    author_email='sjg@chromium.org',
    description='Python binding for libfdt',
    ext_modules=[libfdt_module],
    package_dir={'': objdir},
    py_modules=['libfdt'],

    long_description=long_description,
    long_description_content_type="text/plain",
    url="https://git.kernel.org/pub/scm/utils/dtc/dtc.git",
    license="BSD",
    license_files=["GPL", "BSD-2-Clause"],

    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: BSD License",
        "License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)",
        "Operating System :: OS Independent",
    ],

)
