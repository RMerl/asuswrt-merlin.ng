#!/usr/bin/env python2
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Device tree to C tool

This tool converts a device tree binary file (.dtb) into two C files. The
indent is to allow a C program to access data from the device tree without
having to link against libfdt. By putting the data from the device tree into
C structures, normal C code can be used. This helps to reduce the size of the
compiled program.

Dtoc produces two output files:

   dt-structs.h  - contains struct definitions
   dt-platdata.c - contains data from the device tree using the struct
                      definitions, as well as U-Boot driver definitions.

This tool is used in U-Boot to provide device tree data to SPL without
increasing the code size of SPL. This supports the CONFIG_SPL_OF_PLATDATA
options. For more information about the use of this options and tool please
see doc/driver-model/of-plat.txt
"""

from optparse import OptionParser
import os
import sys
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '../patman'))

# Bring in the libfdt module
sys.path.insert(0, 'scripts/dtc/pylibfdt')
sys.path.insert(0, os.path.join(our_path,
                '../../build-sandbox_spl/scripts/dtc/pylibfdt'))

import dtb_platdata
import test_util

def run_tests(args):
    """Run all the test we have for dtoc

    Args:
        args: List of positional args provided to dtoc. This can hold a test
            name to execute (as in 'dtoc -t test_empty_file', for example)
    """
    import test_dtoc

    result = unittest.TestResult()
    sys.argv = [sys.argv[0]]
    test_name = args and args[0] or None
    for module in (test_dtoc.TestDtoc,):
        if test_name:
            try:
                suite = unittest.TestLoader().loadTestsFromName(test_name, module)
            except AttributeError:
                continue
        else:
            suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    print result
    for _, err in result.errors:
        print err
    for _, err in result.failures:
        print err

def RunTestCoverage():
    """Run the tests and check that we get 100% coverage"""
    sys.argv = [sys.argv[0]]
    test_util.RunTestCoverage('tools/dtoc/dtoc.py', '/dtoc.py',
            ['tools/patman/*.py', '*/fdt*', '*test*'], options.build_dir)


if __name__ != '__main__':
    sys.exit(1)

parser = OptionParser()
parser.add_option('-B', '--build-dir', type='string', default='b',
        help='Directory containing the build output')
parser.add_option('-d', '--dtb-file', action='store',
                  help='Specify the .dtb input file')
parser.add_option('--include-disabled', action='store_true',
                  help='Include disabled nodes')
parser.add_option('-o', '--output', action='store', default='-',
                  help='Select output filename')
parser.add_option('-P', '--processes', type=int,
                  help='set number of processes to use for running tests')
parser.add_option('-t', '--test', action='store_true', dest='test',
                  default=False, help='run tests')
parser.add_option('-T', '--test-coverage', action='store_true',
                default=False, help='run tests and check for 100% coverage')
(options, args) = parser.parse_args()

# Run our meagre tests
if options.test:
    run_tests(args)

elif options.test_coverage:
    RunTestCoverage()

else:
    dtb_platdata.run_steps(args, options.dtb_file, options.include_disabled,
                           options.output)
