# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
#

from contextlib import contextmanager
import glob
import os
import sys

import command

try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO


def RunTestCoverage(prog, filter_fname, exclude_list, build_dir, required=None):
    """Run tests and check that we get 100% coverage

    Args:
        prog: Program to run (with be passed a '-t' argument to run tests
        filter_fname: Normally all *.py files in the program's directory will
            be included. If this is not None, then it is used to filter the
            list so that only filenames that don't contain filter_fname are
            included.
        exclude_list: List of file patterns to exclude from the coverage
            calculation
        build_dir: Build directory, used to locate libfdt.py
        required: List of modules which must be in the coverage report

    Raises:
        ValueError if the code coverage is not 100%
    """
    # This uses the build output from sandbox_spl to get _libfdt.so
    path = os.path.dirname(prog)
    if filter_fname:
        glob_list = glob.glob(os.path.join(path, '*.py'))
        glob_list = [fname for fname in glob_list if filter_fname in fname]
    else:
        glob_list = []
    glob_list += exclude_list
    glob_list += ['*libfdt.py', '*site-packages*']
    cmd = ('PYTHONPATH=$PYTHONPATH:%s/sandbox_spl/tools python-coverage run '
           '--omit "%s" %s -P1 -t' % (build_dir, ','.join(glob_list), prog))
    os.system(cmd)
    stdout = command.Output('python-coverage', 'report')
    lines = stdout.splitlines()
    if required:
        # Convert '/path/to/name.py' just the module name 'name'
        test_set = set([os.path.splitext(os.path.basename(line.split()[0]))[0]
                        for line in lines if '/etype/' in line])
        missing_list = required
        missing_list.difference_update(test_set)
        if missing_list:
            print 'Missing tests for %s' % (', '.join(missing_list))
            print stdout
            ok = False

    coverage = lines[-1].split(' ')[-1]
    ok = True
    print coverage
    if coverage != '100%':
        print stdout
        print ("Type 'python-coverage html' to get a report in "
               'htmlcov/index.html')
        print 'Coverage error: %s, but should be 100%%' % coverage
        ok = False
    if not ok:
        raise ValueError('Test coverage failure')


# Use this to suppress stdout/stderr output:
# with capture_sys_output() as (stdout, stderr)
#   ...do something...
@contextmanager
def capture_sys_output():
    capture_out, capture_err = StringIO(), StringIO()
    old_out, old_err = sys.stdout, sys.stderr
    try:
        sys.stdout, sys.stderr = capture_out, capture_err
        yield capture_out, capture_err
    finally:
        sys.stdout, sys.stderr = old_out, old_err
