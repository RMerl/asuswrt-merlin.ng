#!/usr/bin/env python

"""
Best-practices tracker for Tor source code.

Go through the various .c files and collect metrics about them. If the metrics
violate some of our best practices and they are not found in the optional
exceptions file, then log a problem about them.

We currently do metrics about file size, function size and number of includes,
for C source files and headers.

practracker.py should be run with its second argument pointing to the Tor
top-level source directory like this:
  $ python3 ./scripts/maint/practracker/practracker.py .

To regenerate the exceptions file so that it allows all current
problems in the Tor source, use the --regen flag:
  $ python3 --regen ./scripts/maint/practracker/practracker.py .
"""

# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import codecs, os, sys

import metrics
import util
import problem
import includes
import shutil

# The filename of the exceptions file (it should be placed in the practracker directory)
EXCEPTIONS_FNAME = "./exceptions.txt"

# Recommended file size
MAX_FILE_SIZE = 3000 # lines
# Recommended function size
MAX_FUNCTION_SIZE = 100 # lines
# Recommended number of #includes
MAX_INCLUDE_COUNT = 50
# Recommended file size for headers
MAX_H_FILE_SIZE = 500
# Recommended include count for headers
MAX_H_INCLUDE_COUNT = 15
# Recommended number of dependency violations
MAX_DEP_VIOLATIONS = 0

# Map from problem type to functions that adjust for tolerance
TOLERANCE_FNS = {
    'include-count': lambda n: int(n*1.1),
    'function-size': lambda n: int(n*1.1),
    'file-size': lambda n: int(n*1.02),
    'dependency-violation': lambda n: (n+2)
}

#######################################################

# The Tor source code topdir
TOR_TOPDIR = None

#######################################################

def open_file(fname):
    return codecs.open(fname, 'r', encoding='utf-8')

def consider_file_size(fname, f):
    """Consider the size of 'f' and yield an FileSizeItem for it.
    """
    file_size = metrics.get_file_len(f)
    yield problem.FileSizeItem(fname, file_size)

def consider_includes(fname, f):
    """Consider the #include count in for 'f' and yield an IncludeCountItem
        for it.
    """
    include_count = metrics.get_include_count(f)

    yield problem.IncludeCountItem(fname, include_count)

def consider_function_size(fname, f):
    """yield a FunctionSizeItem for every function in f.
    """

    for name, lines in metrics.get_function_lines(f):
        canonical_function_name = "%s:%s()" % (fname, name)
        yield problem.FunctionSizeItem(canonical_function_name, lines)

def consider_include_violations(fname, real_fname, f):
    n = 0
    for item in includes.consider_include_rules(real_fname, f):
        n += 1
    if n:
        yield problem.DependencyViolationItem(fname, n)


#######################################################

def consider_all_metrics(files_list):
    """Consider metrics for all files, and yield a sequence of problem.Item
       object for those issues."""
    for fname in files_list:
        with open_file(fname) as f:
            for item in consider_metrics_for_file(fname, f):
                yield item

def consider_metrics_for_file(fname, f):
    """
       Yield a sequence of problem.Item objects for all of the metrics in
       'f'.
    """
    real_fname = fname
    # Strip the useless part of the path
    if fname.startswith(TOR_TOPDIR):
        fname = fname[len(TOR_TOPDIR):]

    # Get file length
    for item in consider_file_size(fname, f):
        yield item

    # Consider number of #includes
    f.seek(0)
    for item in consider_includes(fname, f):
        yield item

    # Get function length
    f.seek(0)
    for item in consider_function_size(fname, f):
        yield item

    # Check for "upward" includes
    f.seek(0)
    for item in consider_include_violations(fname, real_fname, f):
        yield item

HEADER="""\
# Welcome to the exceptions file for Tor's best-practices tracker!
#
# Each line of this file represents a single violation of Tor's best
# practices -- typically, a violation that we had before practracker.py
# first existed.
#
# There are three kinds of problems that we recognize right now:
#   function-size -- a function of more than {MAX_FUNCTION_SIZE} lines.
#   file-size -- a .c file of more than {MAX_FILE_SIZE} lines, or a .h
#      file with more than {MAX_H_FILE_SIZE} lines.
#   include-count -- a .c file with more than {MAX_INCLUDE_COUNT} #includes,
#      or a .h file with more than {MAX_H_INCLUDE_COUNT} #includes.
#   dependency-violation -- a file includes a header that it should
#      not, according to an advisory .may_include file.
#
# Each line below represents a single exception that practracker should
# _ignore_. Each line has four parts:
#  1. The word "problem".
#  2. The kind of problem.
#  3. The location of the problem: either a filename, or a
#     filename:functionname pair.
#  4. The magnitude of the problem to ignore.
#
# So for example, consider this line:
#    problem file-size /src/core/or/connection_or.c 3200
#
# It tells practracker to allow the mentioned file to be up to 3200 lines
# long, even though ordinarily it would warn about any file with more than
# {MAX_FILE_SIZE} lines.
#
# You can either edit this file by hand, or regenerate it completely by
# running `make practracker-regen`.
#
# Remember: It is better to fix the problem than to add a new exception!

""".format(**globals())

def main(argv):
    import argparse

    progname = argv[0]
    parser = argparse.ArgumentParser(prog=progname)
    parser.add_argument("--regen", action="store_true",
                        help="Regenerate the exceptions file")
    parser.add_argument("--list-overbroad", action="store_true",
                        help="List over-broad exceptions")
    parser.add_argument("--regen-overbroad", action="store_true",
                        help="Regenerate the exceptions file, "
                             "removing over-broad exceptions.")
    parser.add_argument("--exceptions",
                        help="Override the location for the exceptions file")
    parser.add_argument("--strict", action="store_true",
                        help="Make all warnings into errors")
    parser.add_argument("--terse", action="store_true",
                        help="Do not emit helpful instructions.")
    parser.add_argument("--max-h-file-size", default=MAX_H_FILE_SIZE,
                        help="Maximum lines per .h file")
    parser.add_argument("--max-h-include-count", default=MAX_H_INCLUDE_COUNT,
                        help="Maximum includes per .h file")
    parser.add_argument("--max-file-size", default=MAX_FILE_SIZE,
                        help="Maximum lines per .c file")
    parser.add_argument("--max-include-count", default=MAX_INCLUDE_COUNT,
                        help="Maximum includes per .c file")
    parser.add_argument("--max-function-size", default=MAX_FUNCTION_SIZE,
                        help="Maximum lines per function")
    parser.add_argument("--max-dependency-violations", default=MAX_DEP_VIOLATIONS,
                        help="Maximum number of dependency violations to allow")
    parser.add_argument("--include-dir", action="append",
                        default=["src"],
                        help="A directory (under topdir) to search for source")
    parser.add_argument("topdir", default=".", nargs="?",
                        help="Top-level directory for the tor source")
    args = parser.parse_args(argv[1:])

    global TOR_TOPDIR
    TOR_TOPDIR = args.topdir
    if args.exceptions:
        exceptions_file = args.exceptions
    else:
        exceptions_file = os.path.join(TOR_TOPDIR, "scripts/maint/practracker", EXCEPTIONS_FNAME)

    # 0) Configure our thresholds of "what is a problem actually"
    filt = problem.ProblemFilter()
    filt.addThreshold(problem.FileSizeItem("*.c", int(args.max_file_size)))
    filt.addThreshold(problem.IncludeCountItem("*.c", int(args.max_include_count)))
    filt.addThreshold(problem.FileSizeItem("*.h", int(args.max_h_file_size)))
    filt.addThreshold(problem.IncludeCountItem("*.h", int(args.max_h_include_count)))
    filt.addThreshold(problem.FunctionSizeItem("*.c", int(args.max_function_size)))
    filt.addThreshold(problem.DependencyViolationItem("*.c", int(args.max_dependency_violations)))
    filt.addThreshold(problem.DependencyViolationItem("*.h", int(args.max_dependency_violations)))

    if args.list_overbroad + args.regen + args.regen_overbroad > 1:
        print("Cannot use more than one of --regen, --list-overbroad, and "
              "--regen-overbroad.",
              file=sys.stderr)
        sys.exit(1)

    # 1) Get all the .c files we care about
    files_list = util.get_tor_c_files(TOR_TOPDIR, args.include_dir)

    # 2) Initialize problem vault and load an optional exceptions file so that
    # we don't warn about the past
    if args.regen:
        tmpname = exceptions_file + ".tmp"
        tmpfile = open(tmpname, "w")
        problem_file = tmpfile
        problem_file.write(HEADER)
        ProblemVault = problem.ProblemVault()
    else:
        ProblemVault = problem.ProblemVault(exceptions_file)
        problem_file = sys.stdout

    if args.list_overbroad or args.regen_overbroad:
        # If we're looking for overbroad exceptions, don't list problems
        # immediately to the problem file.
        problem_file = util.NullFile()

    # 2.1) Adjust the exceptions so that we warn only about small problems,
    # and produce errors on big ones.
    if not (args.regen or args.list_overbroad or args.regen_overbroad or
            args.strict):
        ProblemVault.set_tolerances(TOLERANCE_FNS)

    # 3) Go through all the files and report problems if they are not exceptions
    found_new_issues = 0
    for item in filt.filter(consider_all_metrics(files_list)):
        status = ProblemVault.register_problem(item)
        if status == problem.STATUS_ERR:
            print(item, file=problem_file)
            found_new_issues += 1
        elif status == problem.STATUS_WARN:
            # warnings always go to stdout.
            print("(warning) {}".format(item))

    if args.regen:
        tmpfile.close()
        shutil.move(tmpname, exceptions_file)
        sys.exit(0)

    if args.regen_overbroad:
        tmpname = exceptions_file + ".tmp"
        tmpfile = open(tmpname, "w")
        tmpfile.write(HEADER)
        for item in ProblemVault.list_exceptions_without_overbroad():
            print(item, file=tmpfile)
        tmpfile.close()
        shutil.move(tmpname, exceptions_file)
        sys.exit(0)

    # If new issues were found, try to give out some advice to the developer on how to resolve it.
    if found_new_issues and not args.regen and not args.terse:
        new_issues_str = """\
FAILURE: practracker found {} new problem(s) in the code: see warnings above.

Please fix the problems if you can, and update the exceptions file
({}) if you can't.

See doc/HACKING/HelpfulTools.md for more information on using practracker.\

You can disable this message by setting the TOR_DISABLE_PRACTRACKER environment
variable.
""".format(found_new_issues, exceptions_file)
        print(new_issues_str)

    if args.list_overbroad:
        def k_fn(tup):
            return tup[0].key()
        for (ex,p) in sorted(ProblemVault.list_overbroad_exceptions(), key=k_fn):
            if p is None:
                print(ex, "->", 0)
            else:
                print(ex, "->", p.metric_value)


    sys.exit(found_new_issues)

if __name__ == '__main__':
    if os.environ.get("TOR_DISABLE_PRACTRACKER"):
        print("TOR_DISABLE_PRACTRACKER is set, skipping practracker tests.",
              file=sys.stderr)
        sys.exit(0)
    main(sys.argv)
