#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2023 Google LLC
#

import os
import sys

if __name__ == "__main__":
    # Allow 'from u_boot_pylib import xxx to work'
    our_path = os.path.dirname(os.path.realpath(__file__))
    sys.path.append(os.path.join(our_path, '..'))

    # Run tests
    from u_boot_pylib import terminal
    from u_boot_pylib import test_util

    result = test_util.run_test_suites(
        'u_boot_pylib', False, False, False, None, None, None,
        ['terminal'])

    sys.exit(0 if result.wasSuccessful() else 1)
