#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#

"""See README for more information"""

try:
    from importlib import resources
except ImportError:
    # for Python 3.6
    import importlib_resources as resources
import os
import re
import sys
import traceback

# Allow 'from patman import xxx to work'
# pylint: disable=C0413
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '..'))

# Our modules
from patman import cmdline
from patman import control
from u_boot_pylib import terminal
from u_boot_pylib import test_util
from u_boot_pylib import tools


def run_patman():
    """Run patamn

    This is the main program. It collects arguments and runs either the tests or
    the control module.
    """
    args = cmdline.parse_args()

    if not args.debug:
        sys.tracebacklimit = 0

    # Run our meagre tests
    if args.cmd == 'test':
        # pylint: disable=C0415
        from patman import func_test
        from patman import test_checkpatch

        result = test_util.run_test_suites(
            'patman', False, False, False, None, None, None,
            [test_checkpatch.TestPatch, func_test.TestFunctional,
             'gitutil', 'settings'])

        sys.exit(0 if result.wasSuccessful() else 1)

    # Process commits, produce patches files, check them, email them
    elif args.cmd == 'send':
        # Called from git with a patch filename as argument
        # Printout a list of additional CC recipients for this patch
        if args.cc_cmd:
            re_line = re.compile(r'(\S*) (.*)')
            with open(args.cc_cmd, 'r', encoding='utf-8') as inf:
                for line in inf.readlines():
                    match = re_line.match(line)
                    if match and match.group(1) == args.patchfiles[0]:
                        for cca in match.group(2).split('\0'):
                            cca = cca.strip()
                            if cca:
                                print(cca)

        elif args.full_help:
            with resources.path('patman', 'README.rst') as readme:
                tools.print_full_help(str(readme))
        else:
            # If we are not processing tags, no need to warning about bad ones
            if not args.process_tags:
                args.ignore_bad_tags = True
            control.send(args)

    # Check status of patches in patchwork
    elif args.cmd == 'status':
        ret_code = 0
        try:
            control.patchwork_status(args.branch, args.count, args.start, args.end,
                                     args.dest_branch, args.force,
                                     args.show_comments, args.patchwork_url)
        except Exception as exc:
            terminal.tprint(f'patman: {type(exc).__name__}: {exc}',
                            colour=terminal.Color.RED)
            if args.debug:
                print()
                traceback.print_exc()
            ret_code = 1
        sys.exit(ret_code)


if __name__ == "__main__":
    sys.exit(run_patman())
