# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
#

from contextlib import contextmanager
import doctest
import glob
import multiprocessing
import os
import sys
import unittest

from u_boot_pylib import command

from io import StringIO

use_concurrent = True
try:
    from concurrencytest import ConcurrentTestSuite
    from concurrencytest import fork_for_tests
except:
    use_concurrent = False


def run_test_coverage(prog, filter_fname, exclude_list, build_dir, required=None,
                    extra_args=None, single_thread='-P1'):
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
        extra_args (str): Extra arguments to pass to the tool before the -t/test
            arg
        single_thread (str): Argument string to make the tests run
            single-threaded. This is necessary to get proper coverage results.
            The default is '-P0'

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
    glob_list += ['*libfdt.py', '*site-packages*', '*dist-packages*']
    glob_list += ['*concurrencytest*']
    test_cmd = 'test' if 'binman' in prog or 'patman' in prog else '-t'
    prefix = ''
    if build_dir:
        prefix = 'PYTHONPATH=$PYTHONPATH:%s/sandbox_spl/tools ' % build_dir
    cmd = ('%spython3-coverage run '
           '--omit "%s" %s %s %s %s' % (prefix, ','.join(glob_list),
                                        prog, extra_args or '', test_cmd,
                                        single_thread or '-P1'))
    os.system(cmd)
    stdout = command.output('python3-coverage', 'report')
    lines = stdout.splitlines()
    if required:
        # Convert '/path/to/name.py' just the module name 'name'
        test_set = set([os.path.splitext(os.path.basename(line.split()[0]))[0]
                        for line in lines if '/etype/' in line])
        missing_list = required
        missing_list.discard('__init__')
        missing_list.difference_update(test_set)
        if missing_list:
            print('Missing tests for %s' % (', '.join(missing_list)))
            print(stdout)
            ok = False

    coverage = lines[-1].split(' ')[-1]
    ok = True
    print(coverage)
    if coverage != '100%':
        print(stdout)
        print("To get a report in 'htmlcov/index.html', type: python3-coverage html")
        print('Coverage error: %s, but should be 100%%' % coverage)
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


class FullTextTestResult(unittest.TextTestResult):
    """A test result class that can print extended text results to a stream

    This is meant to be used by a TestRunner as a result class. Like
    TextTestResult, this prints out the names of tests as they are run,
    errors as they occur, and a summary of the results at the end of the
    test run. Beyond those, this prints information about skipped tests,
    expected failures and unexpected successes.

    Args:
        stream: A file-like object to write results to
        descriptions (bool): True to print descriptions with test names
        verbosity (int): Detail of printed output per test as they run
            Test stdout and stderr always get printed when buffering
            them is disabled by the test runner. In addition to that,
            0: Print nothing
            1: Print a dot per test
            2: Print test names
    """
    def __init__(self, stream, descriptions, verbosity):
        self.verbosity = verbosity
        super().__init__(stream, descriptions, verbosity)

    def printErrors(self):
        "Called by TestRunner after test run to summarize the tests"
        # The parent class doesn't keep unexpected successes in the same
        # format as the rest. Adapt it to what printErrorList expects.
        unexpected_successes = [
            (test, 'Test was expected to fail, but succeeded.\n')
            for test in self.unexpectedSuccesses
        ]

        super().printErrors()  # FAIL and ERROR
        self.printErrorList('SKIP', self.skipped)
        self.printErrorList('XFAIL', self.expectedFailures)
        self.printErrorList('XPASS', unexpected_successes)

    def addSkip(self, test, reason):
        """Called when a test is skipped."""
        # Add empty line to keep spacing consistent with other results
        if not reason.endswith('\n'):
            reason += '\n'
        super().addSkip(test, reason)


def run_test_suites(toolname, debug, verbosity, test_preserve_dirs, processes,
                    test_name, toolpath, class_and_module_list):
    """Run a series of test suites and collect the results

    Args:
        toolname: Name of the tool that ran the tests
        debug: True to enable debugging, which shows a full stack trace on error
        verbosity: Verbosity level to use (0-4)
        test_preserve_dirs: True to preserve the input directory used by tests
            so that it can be examined afterwards (only useful for debugging
            tests). If a single test is selected (in args[0]) it also preserves
            the output directory for this test. Both directories are displayed
            on the command line.
        processes: Number of processes to use to run tests (None=same as #CPUs)
        test_name: Name of test to run, or None for all
        toolpath: List of paths to use for tools
        class_and_module_list: List of test classes (type class) and module
           names (type str) to run
    """
    sys.argv = [sys.argv[0]]
    if debug:
        sys.argv.append('-D')
    if verbosity:
        sys.argv.append('-v%d' % verbosity)
    if toolpath:
        for path in toolpath:
            sys.argv += ['--toolpath', path]

    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    runner = unittest.TextTestRunner(
        stream=sys.stdout,
        verbosity=(1 if verbosity is None else verbosity),
        resultclass=FullTextTestResult,
    )

    if use_concurrent and processes != 1:
        suite = ConcurrentTestSuite(suite,
                fork_for_tests(processes or multiprocessing.cpu_count()))

    for module in class_and_module_list:
        if isinstance(module, str) and (not test_name or test_name == module):
            suite.addTests(doctest.DocTestSuite(module))

    for module in class_and_module_list:
        if isinstance(module, str):
            continue
        # Test the test module about our arguments, if it is interested
        if hasattr(module, 'setup_test_args'):
            setup_test_args = getattr(module, 'setup_test_args')
            setup_test_args(preserve_indir=test_preserve_dirs,
                preserve_outdirs=test_preserve_dirs and test_name is not None,
                toolpath=toolpath, verbosity=verbosity)
        if test_name:
            # Since Python v3.5 If an ImportError or AttributeError occurs
            # while traversing a name then a synthetic test that raises that
            # error when run will be returned. Check that the requested test
            # exists, otherwise these errors are included in the results.
            if test_name in loader.getTestCaseNames(module):
                suite.addTests(loader.loadTestsFromName(test_name, module))
        else:
            suite.addTests(loader.loadTestsFromTestCase(module))

    print(f" Running {toolname} tests ".center(70, "="))
    result = runner.run(suite)
    print()

    return result
