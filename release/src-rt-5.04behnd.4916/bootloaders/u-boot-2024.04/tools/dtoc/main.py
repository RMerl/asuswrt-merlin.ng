#!/usr/bin/env python3
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

Dtoc produces several output files - see OUTPUT_FILES in dtb_platdata.py

This tool is used in U-Boot to provide device tree data to SPL without
increasing the code size of SPL. This supports the CONFIG_SPL_OF_PLATDATA
options. For more information about the use of this options and tool please
see doc/driver-model/of-plat.rst
"""

from argparse import ArgumentParser
import os
import pathlib
import sys

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '..'))

# Bring in the libfdt module
sys.path.insert(0, 'scripts/dtc/pylibfdt')
sys.path.insert(0, os.path.join(our_path,
                '../../build-sandbox_spl/scripts/dtc/pylibfdt'))

from dtoc import dtb_platdata
from u_boot_pylib import test_util

DTOC_DIR = pathlib.Path(__file__).parent
HAVE_TESTS = (DTOC_DIR / 'test_dtoc.py').exists()

def run_tests(processes, args):
    """Run all the test we have for dtoc

    Args:
        processes: Number of processes to use to run tests (None=same as #CPUs)
        args: List of positional args provided to dtoc. This can hold a test
            name to execute (as in 'dtoc -t test_empty_file', for example)
    """
    from dtoc import test_src_scan
    from dtoc import test_dtoc

    sys.argv = [sys.argv[0]]
    test_name = args.files and args.files[0] or None

    test_dtoc.setup()

    result = test_util.run_test_suites(
        toolname='dtoc', debug=True, verbosity=1, test_preserve_dirs=False,
        processes=processes, test_name=test_name, toolpath=[],
        class_and_module_list=[test_dtoc.TestDtoc,test_src_scan.TestSrcScan])

    return (0 if result.wasSuccessful() else 1)


def RunTestCoverage(build_dir):
    """Run the tests and check that we get 100% coverage"""
    sys.argv = [sys.argv[0]]
    test_util.run_test_coverage('tools/dtoc/dtoc', '/main.py',
            ['tools/patman/*.py', 'tools/u_boot_pylib/*','*/fdt*', '*test*'],
            build_dir)


def run_dtoc():
    epilog = 'Generate C code from devicetree files. See of-plat.rst for details'

    parser = ArgumentParser(epilog=epilog)
    parser.add_argument('-B', '--build-dir', type=str, default='b',
            help='Directory containing the build output')
    parser.add_argument('-c', '--c-output-dir', action='store',
                      help='Select output directory for C files')
    parser.add_argument(
        '-C', '--h-output-dir', action='store',
        help='Select output directory for H files (defaults to --c-output-di)')
    parser.add_argument('-d', '--dtb-file', action='store',
                      help='Specify the .dtb input file')
    parser.add_argument(
        '-i', '--instantiate', action='store_true', default=False,
        help='Instantiate devices to avoid needing device_bind()')
    parser.add_argument('--include-disabled', action='store_true',
                      help='Include disabled nodes')
    parser.add_argument('-o', '--output', action='store',
                      help='Select output filename')
    parser.add_argument(
        '-p', '--phase', type=str,
        help='set phase of U-Boot this invocation is for (spl/tpl)')
    parser.add_argument('-P', '--processes', type=int,
                      help='set number of processes to use for running tests')
    if HAVE_TESTS:
        parser.add_argument('-t', '--test', action='store_true', dest='test',
                            default=False, help='run tests')
        parser.add_argument(
            '-T', '--test-coverage', action='store_true',
            default=False, help='run tests and check for 100%% coverage')
    parser.add_argument('files', nargs='*')
    args = parser.parse_args()

    # Run our meagre tests
    if HAVE_TESTS and args.test:
        ret_code = run_tests(args.processes, args)
        sys.exit(ret_code)

    elif HAVE_TESTS and args.test_coverage:
        RunTestCoverage(args.build_dir)

    else:
        dtb_platdata.run_steps(args.files, args.dtb_file, args.include_disabled,
                               args.output,
                               [args.c_output_dir, args.h_output_dir],
                               args.phase, instantiate=args.instantiate)


if __name__ == '__main__':
    run_dtoc()
