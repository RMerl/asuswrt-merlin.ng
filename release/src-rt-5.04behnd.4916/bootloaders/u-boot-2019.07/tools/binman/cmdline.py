# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Command-line parser for binman
#

from optparse import OptionParser

def ParseArgs(argv):
    """Parse the binman command-line arguments

    Args:
        argv: List of string arguments
    Returns:
        Tuple (options, args) with the command-line options and arugments.
            options provides access to the options (e.g. option.debug)
            args is a list of string arguments
    """
    parser = OptionParser()
    parser.add_option('-a', '--entry-arg', type='string', action='append',
            help='Set argument value arg=value')
    parser.add_option('-b', '--board', type='string',
            help='Board name to build')
    parser.add_option('-B', '--build-dir', type='string', default='b',
            help='Directory containing the build output')
    parser.add_option('-d', '--dt', type='string',
            help='Configuration file (.dtb) to use')
    parser.add_option('-D', '--debug', action='store_true',
            help='Enabling debugging (provides a full traceback on error)')
    parser.add_option('-E', '--entry-docs', action='store_true',
            help='Write out entry documentation (see README.entries)')
    parser.add_option('--fake-dtb', action='store_true',
            help='Use fake device tree contents (for testing only)')
    parser.add_option('-i', '--image', type='string', action='append',
            help='Image filename to build (if not specified, build all)')
    parser.add_option('-I', '--indir', action='append',
            help='Add a path to a directory to use for input files')
    parser.add_option('-H', '--full-help', action='store_true',
        default=False, help='Display the README file')
    parser.add_option('-m', '--map', action='store_true',
        default=False, help='Output a map file for each image')
    parser.add_option('-O', '--outdir', type='string',
        action='store', help='Path to directory to use for intermediate and '
        'output files')
    parser.add_option('-p', '--preserve', action='store_true',\
        help='Preserve temporary output directory even if option -O is not '
             'given')
    parser.add_option('-P', '--processes', type=int,
                      help='set number of processes to use for running tests')
    parser.add_option('-t', '--test', action='store_true',
                    default=False, help='run tests')
    parser.add_option('-T', '--test-coverage', action='store_true',
                    default=False, help='run tests and check for 100% coverage')
    parser.add_option('-u', '--update-fdt', action='store_true',
        default=False, help='Update the binman node with offset/size info')
    parser.add_option('-v', '--verbosity', default=1,
        type='int', help='Control verbosity: 0=silent, 1=progress, 3=full, '
        '4=debug')

    parser.usage += """

Create images for a board from a set of binaries. It is controlled by a
description in the board device tree."""

    return parser.parse_args(argv)
