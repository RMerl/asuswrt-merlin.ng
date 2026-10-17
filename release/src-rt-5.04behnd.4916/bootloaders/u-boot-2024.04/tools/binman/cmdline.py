# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Command-line parser for binman"""

import argparse
from argparse import ArgumentParser
import os
from binman import state
import os
import pathlib

BINMAN_DIR = pathlib.Path(__file__).parent
HAS_TESTS = (BINMAN_DIR / "ftest.py").exists()

def make_extract_parser(subparsers):
    """make_extract_parser: Make a subparser for the 'extract' command

    Args:
        subparsers (ArgumentParser): parser to add this one to
    """
    extract_parser = subparsers.add_parser('extract',
                                           help='Extract files from an image')
    extract_parser.add_argument('-F', '--format', type=str,
        help='Select an alternative format for extracted data')
    extract_parser.add_argument('-i', '--image', type=str, required=True,
                                help='Image filename to extract')
    extract_parser.add_argument('-f', '--filename', type=str,
                                help='Output filename to write to')
    extract_parser.add_argument('-O', '--outdir', type=str, default='',
        help='Path to directory to use for output files')
    extract_parser.add_argument('paths', type=str, nargs='*',
                                help='Paths within file to extract (wildcard)')
    extract_parser.add_argument('-U', '--uncompressed', action='store_true',
        help='Output raw uncompressed data for compressed entries')


#pylint: disable=R0903
class BinmanVersion(argparse.Action):
    """Handles the -V option to binman

    This reads the version information from a file called 'version' in the same
    directory as this file.

    If not present it assumes this is running from the U-Boot tree and collects
    the version from the Makefile.

    The format of the version information is three VAR = VALUE lines, for
    example:

        VERSION = 2022
        PATCHLEVEL = 01
        EXTRAVERSION = -rc2
    """
    def __init__(self, nargs=0, **kwargs):
        super().__init__(nargs=nargs, **kwargs)

    def __call__(self, parser, namespace, values, option_string=None):
        parser._print_message(f'Binman {state.GetVersion()}\n')
        parser.exit()


def ParseArgs(argv):
    """Parse the binman command-line arguments

    Args:
        argv (list of str): List of string arguments

    Returns:
        tuple: (options, args) with the command-line options and arugments.
            options provides access to the options (e.g. option.debug)
            args is a list of string arguments
    """
    def _AddPreserve(pars):
        pars.add_argument('-O', '--outdir', type=str,
            action='store', help='Path to directory to use for intermediate '
            'and output files')
        pars.add_argument('-p', '--preserve', action='store_true',\
            help='Preserve temporary output directory even if option -O is not '
                 'given')

    if '-H' in argv:
        argv.append('build')

    epilog = '''Binman creates and manipulate images for a board from a set of binaries. Binman is
controlled by a description in the board device tree.'''

    parser = ArgumentParser(epilog=epilog)
    parser.add_argument('-B', '--build-dir', type=str, default='b',
        help='Directory containing the build output')
    parser.add_argument('-D', '--debug', action='store_true',
        help='Enabling debugging (provides a full traceback on error)')
    parser.add_argument('-H', '--full-help', action='store_true',
        default=False, help='Display the README file')
    parser.add_argument('--tooldir', type=str,
        default=os.path.join(os.path.expanduser('~/.binman-tools')),
        help='Set the directory to store tools')
    parser.add_argument('--toolpath', type=str, action='append',
        help='Add a path to the list of directories containing tools')
    parser.add_argument('-T', '--threads', type=int,
          default=None, help='Number of threads to use (0=single-thread)')
    parser.add_argument('--test-section-timeout', action='store_true',
          help='Use a zero timeout for section multi-threading (for testing)')
    parser.add_argument('-v', '--verbosity', default=1,
        type=int, help='Control verbosity: 0=silent, 1=warnings, 2=notices, '
        '3=info, 4=detail, 5=debug')
    parser.add_argument('-V', '--version', nargs=0, action=BinmanVersion)

    subparsers = parser.add_subparsers(dest='cmd')
    subparsers.required = True

    build_parser = subparsers.add_parser('build', help='Build firmware image')
    build_parser.add_argument('-a', '--entry-arg', type=str, action='append',
            help='Set argument value arg=value')
    build_parser.add_argument('-b', '--board', type=str,
            help='Board name to build')
    build_parser.add_argument('-d', '--dt', type=str,
            help='Configuration file (.dtb) to use')
    build_parser.add_argument('--fake-dtb', action='store_true',
            help='Use fake device tree contents (for testing only)')
    build_parser.add_argument('--fake-ext-blobs', action='store_true',
            help='Create fake ext blobs with dummy content (for testing only)')
    build_parser.add_argument('--force-missing-bintools', type=str,
            help='Comma-separated list of bintools to consider missing (for testing)')
    build_parser.add_argument('-i', '--image', type=str, action='append',
            help='Image filename to build (if not specified, build all)')
    build_parser.add_argument('-I', '--indir', action='append',
            help='Add a path to the list of directories to use for input files')
    build_parser.add_argument('-m', '--map', action='store_true',
        default=False, help='Output a map file for each image')
    build_parser.add_argument('-M', '--allow-missing', action='store_true',
        default=False, help='Allow external blobs and bintools to be missing')
    build_parser.add_argument('-n', '--no-expanded', action='store_true',
            help="Don't use 'expanded' versions of entries where available; "
                 "normally 'u-boot' becomes 'u-boot-expanded', for example")
    _AddPreserve(build_parser)
    build_parser.add_argument('-u', '--update-fdt', action='store_true',
        default=False, help='Update the binman node with offset/size info')
    build_parser.add_argument('--update-fdt-in-elf', type=str,
        help='Update an ELF file with the output dtb: infile,outfile,begin_sym,end_sym')
    build_parser.add_argument(
        '-W', '--ignore-missing', action='store_true', default=False,
        help='Return success even if there are missing blobs/bintools (requires -M)')

    subparsers.add_parser(
        'bintool-docs', help='Write out bintool documentation (see bintool.rst)')

    subparsers.add_parser(
        'entry-docs', help='Write out entry documentation (see entries.rst)')

    list_parser = subparsers.add_parser('ls', help='List files in an image')
    list_parser.add_argument('-i', '--image', type=str, required=True,
                             help='Image filename to list')
    list_parser.add_argument('paths', type=str, nargs='*',
                             help='Paths within file to list (wildcard)')

    make_extract_parser(subparsers)

    replace_parser = subparsers.add_parser('replace',
                                           help='Replace entries in an image')
    replace_parser.add_argument('-C', '--compressed', action='store_true',
        help='Input data is already compressed if needed for the entry')
    replace_parser.add_argument('-i', '--image', type=str, required=True,
                                help='Image filename to update')
    replace_parser.add_argument('-f', '--filename', type=str,
                                help='Input filename to read from')
    replace_parser.add_argument('-F', '--fix-size', action='store_true',
        help="Don't allow entries to be resized")
    replace_parser.add_argument('-I', '--indir', type=str, default='',
        help='Path to directory to use for input files')
    replace_parser.add_argument('-m', '--map', action='store_true',
        default=False, help='Output a map file for the updated image')
    _AddPreserve(replace_parser)
    replace_parser.add_argument('paths', type=str, nargs='*',
                                help='Paths within file to replace (wildcard)')

    sign_parser = subparsers.add_parser('sign',
                                           help='Sign entries in image')
    sign_parser.add_argument('-a', '--algo', type=str, required=True,
                                help='Hash algorithm e.g. sha256,rsa4096')
    sign_parser.add_argument('-f', '--file', type=str, required=False,
                                help='Input filename to sign')
    sign_parser.add_argument('-i', '--image', type=str, required=True,
                                help='Image filename to update')
    sign_parser.add_argument('-k', '--key', type=str, required=True,
                                help='Private key file for signing')
    sign_parser.add_argument('paths', type=str, nargs='*',
                                help='Paths within file to sign (wildcard)')

    if HAS_TESTS:
        test_parser = subparsers.add_parser('test', help='Run tests')
        test_parser.add_argument('-P', '--processes', type=int,
            help='set number of processes to use for running tests')
        test_parser.add_argument('-T', '--test-coverage', action='store_true',
            default=False, help='run tests and check for 100%% coverage')
        test_parser.add_argument(
            '-X', '--test-preserve-dirs', action='store_true',
            help='Preserve and display test-created input directories; also '
                 'preserve the output directory if a single test is run (pass '
                 'test name at the end of the command line')
        test_parser.add_argument('tests', nargs='*',
                                 help='Test names to run (omit for all)')

    tool_parser = subparsers.add_parser('tool', help='Check bintools')
    tool_parser.add_argument('-l', '--list', action='store_true',
                             help='List all known bintools')
    tool_parser.add_argument(
        '-f', '--fetch', action='store_true',
        help='fetch a bintool from a known location (or: all/missing)')
    tool_parser.add_argument('bintools', type=str, nargs='*')

    return parser.parse_args(argv)
