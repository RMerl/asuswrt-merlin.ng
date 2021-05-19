# Copyright 2013-2019, The Tor Project, Inc
# See LICENSE for licensing information

"""
bt_test.py

This file tests the output from test-bt-cl to make sure it's as expected.

Example usage:

$ ./src/test/test-bt-cl crash | ./src/test/bt_test.py
OK
$ ./src/test/test-bt-cl assert | ./src/test/bt_test.py
OK

"""

# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import sys


def matches(lines, funcs):
    if len(lines) < len(funcs):
        return False
    try:
        for l, f in zip(lines, funcs):
            l.index(f)
    except ValueError:
        return False
    else:
        return True

FUNCNAMES = "crash oh_what a_tangled_web we_weave main".split()

LINES = sys.stdin.readlines()

for I in range(len(LINES)):
    if matches(LINES[I:], FUNCNAMES):
        print("OK")
        sys.exit(0)

print("BAD")

for l in LINES:
    print("{}".format(l), end="")

if (sys.platform.startswith('freebsd') or sys.platform.startswith('netbsd') or
    sys.platform.startswith('openbsd') or sys.platform.startswith('darwin')):
    # See bug #17808 if you know how to fix backtraces on BSD-derived systems
    print("Test failed; but {} is known to have backtrace problems."
          .format(sys.platform))
    print("Treating as 'SKIP'.")
    sys.exit(77)

sys.exit(1)
