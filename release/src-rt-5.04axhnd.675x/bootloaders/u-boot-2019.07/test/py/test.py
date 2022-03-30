#!/usr/bin/env python2
# SPDX-License-Identifier: GPL-2.0

# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Wrapper script to invoke pytest with the directory name that contains the
# U-Boot tests.

from __future__ import print_function

import os
import os.path
import sys

# Get rid of argv[0]
sys.argv.pop(0)

# argv; py.test test_directory_name user-supplied-arguments
args = ['py.test', os.path.dirname(__file__) + '/tests']
args.extend(sys.argv)

try:
    os.execvp('py.test', args)
except:
    # Log full details of any exception for detailed analysis
    import traceback
    traceback.print_exc()
    # Hint to the user that they likely simply haven't installed the required
    # dependencies.
    print('''
exec(py.test) failed; perhaps you are missing some dependencies?
See test/py/README.md for the list.''', file=sys.stderr)
    sys.exit(1)
