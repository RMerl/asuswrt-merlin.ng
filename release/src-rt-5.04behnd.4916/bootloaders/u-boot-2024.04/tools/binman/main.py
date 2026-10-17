#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+

# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

"""See README for more information"""

import os
import site
import sys
import traceback

# Get the absolute path to this file at run-time
our_path = os.path.dirname(os.path.realpath(__file__))
our1_path = os.path.dirname(our_path)
our2_path = os.path.dirname(our1_path)

# Extract $(srctree) from Kbuild environment, or use relative paths below
srctree = os.environ.get('srctree', our2_path)

#
# Do not pollute source tree with cache files:
# https://stackoverflow.com/a/60024195/2511795
# https://bugs.python.org/issue33499
#
sys.pycache_prefix = os.path.relpath(our_path, srctree)

# Bring in the patman and dtoc libraries (but don't override the first path
# in PYTHONPATH)
sys.path.insert(2, our1_path)

from binman import bintool
from u_boot_pylib import test_util

# Bring in the libfdt module
sys.path.insert(2, 'scripts/dtc/pylibfdt')
sys.path.insert(2, os.path.join(srctree, 'scripts/dtc/pylibfdt'))
sys.path.insert(2, os.path.join(srctree, 'build-sandbox/scripts/dtc/pylibfdt'))
sys.path.insert(2, os.path.join(srctree, 'build-sandbox_spl/scripts/dtc/pylibfdt'))

from binman import cmdline
from binman import control
from u_boot_pylib import test_util

def RunTests(debug, verbosity, processes, test_preserve_dirs, args, toolpath):
    """Run the functional tests and any embedded doctests

    Args:
        debug: True to enable debugging, which shows a full stack trace on error
        verbosity: Verbosity level to use
        test_preserve_dirs: True to preserve the input directory used by tests
            so that it can be examined afterwards (only useful for debugging
            tests). If a single test is selected (in args[0]) it also preserves
            the output directory for this test. Both directories are displayed
            on the command line.
        processes: Number of processes to use to run tests (None=same as #CPUs)
        args: List of positional args provided to binman. This can hold a test
            name to execute (as in 'binman test testSections', for example)
        toolpath: List of paths to use for tools
    """
    from binman import bintool_test
    from binman import cbfs_util_test
    from binman import elf_test
    from binman import entry_test
    from binman import fdt_test
    from binman import fip_util_test
    from binman import ftest
    from binman import image_test
    import doctest

    test_name = args and args[0] or None

    # Run the entry tests first ,since these need to be the first to import the
    # 'entry' module.
    result = test_util.run_test_suites(
        'binman', debug, verbosity, test_preserve_dirs, processes, test_name,
        toolpath,
        [bintool_test.TestBintool, entry_test.TestEntry, ftest.TestFunctional,
         fdt_test.TestFdt, elf_test.TestElf, image_test.TestImage,
         cbfs_util_test.TestCbfs, fip_util_test.TestFip])

    return (0 if result.wasSuccessful() else 1)

def RunTestCoverage(toolpath, build_dir):
    """Run the tests and check that we get 100% coverage"""
    glob_list = control.GetEntryModules(False)
    all_set = set([os.path.splitext(os.path.basename(item))[0]
                   for item in glob_list if '_testing' not in item])
    extra_args = ''
    if toolpath:
        for path in toolpath:
            extra_args += ' --toolpath %s' % path
    test_util.run_test_coverage('tools/binman/binman', None,
            ['*test*', '*main.py', 'tools/patman/*', 'tools/dtoc/*',
             'tools/u_boot_pylib/*'],
            build_dir, all_set, extra_args or None)

def RunBinman(args):
    """Main entry point to binman once arguments are parsed

    Args:
        args: Command line arguments Namespace object
    """
    ret_code = 0

    if not args.debug:
        sys.tracebacklimit = 0

    # Provide a default toolpath in the hope of finding a mkimage built from
    # current source
    if not args.toolpath:
        args.toolpath = ['./tools', 'build-sandbox/tools']

    if args.cmd == 'test':
        if args.test_coverage:
            RunTestCoverage(args.toolpath, args.build_dir)
        else:
            ret_code = RunTests(args.debug, args.verbosity, args.processes,
                                args.test_preserve_dirs, args.tests,
                                args.toolpath)

    elif args.cmd == 'bintool-docs':
        control.write_bintool_docs(bintool.Bintool.get_tool_list())

    elif args.cmd == 'entry-docs':
        control.WriteEntryDocs(control.GetEntryModules())

    else:
        try:
            ret_code = control.Binman(args)
        except Exception as e:
            print('binman: %s' % e, file=sys.stderr)
            if args.debug:
                print()
                traceback.print_exc()
            ret_code = 1
    return ret_code


def start_binman():
    args = cmdline.ParseArgs(sys.argv[1:])

    ret_code = RunBinman(args)
    sys.exit(ret_code)


if __name__ == "__main__":
    start_binman()
