# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2014 Google, Inc
#

"""Handles parsing of buildman arguments

This creates the argument parser and uses it to parse the arguments passed in
"""

import argparse
import os
import pathlib

BUILDMAN_DIR = pathlib.Path(__file__).parent
HAS_TESTS = os.path.exists(BUILDMAN_DIR / "test.py")

def add_upto_m(parser):
    """Add arguments up to 'M'

    Args:
        parser (ArgumentParser): Parse to add to

    This is split out to avoid having too many statements in one function
    """
    parser.add_argument('-a', '--adjust-cfg', type=str, action='append',
          help='Adjust the Kconfig settings in .config before building')
    parser.add_argument('-A', '--print-prefix', action='store_true',
          help='Print the tool-chain prefix for a board (CROSS_COMPILE=)')
    parser.add_argument('-b', '--branch', type=str,
          help='Branch name to build, or range of commits to build')
    parser.add_argument('-B', '--bloat', dest='show_bloat',
          action='store_true', default=False,
          help='Show changes in function code size for each board')
    parser.add_argument('--boards', type=str, action='append',
          help='List of board names to build separated by comma')
    parser.add_argument('-c', '--count', dest='count', type=int,
          default=-1, help='Run build on the top n commits')
    parser.add_argument('-C', '--force-reconfig', dest='force_reconfig',
          action='store_true', default=False,
          help='Reconfigure for every commit (disable incremental build)')
    parser.add_argument('--config-only', action='store_true',
                        default=False,
                        help="Don't build, just configure each commit")
    parser.add_argument('-d', '--detail', dest='show_detail',
          action='store_true', default=False,
          help='Show detailed size delta for each board in the -S summary')
    parser.add_argument('-D', '--debug', action='store_true',
        help='Enabling debugging (provides a full traceback on error)')
    parser.add_argument('-e', '--show_errors', action='store_true',
          default=False, help='Show errors and warnings')
    parser.add_argument('-E', '--warnings-as-errors', action='store_true',
          default=False, help='Treat all compiler warnings as errors')
    parser.add_argument('-f', '--force-build', dest='force_build',
          action='store_true', default=False,
          help='Force build of boards even if already built')
    parser.add_argument('-F', '--force-build-failures', dest='force_build_failures',
          action='store_true', default=False,
          help='Force build of previously-failed build')
    parser.add_argument('--fetch-arch', type=str,
          help="Fetch a toolchain for architecture FETCH_ARCH ('list' to list)."
              ' You can also fetch several toolchains separate by comma, or'
              " 'all' to download all")
    parser.add_argument(
          '--full-check', action='store_true',
          help='Check maintainer entries and TARGET configs')
    parser.add_argument('-g', '--git', type=str,
          help='Git repo containing branch to build', default='.')
    parser.add_argument('-G', '--config-file', type=str,
          help='Path to buildman config file', default='')
    parser.add_argument('-H', '--full-help', action='store_true', dest='full_help',
          default=False, help='Display the README file')
    parser.add_argument('-i', '--in-tree', dest='in_tree',
          action='store_true', default=False,
          help='Build in the source tree instead of a separate directory')
    parser.add_argument('-I', '--ide', action='store_true', default=False,
          help='Create build output that can be parsed by an IDE')
    parser.add_argument('-j', '--jobs', dest='jobs', type=int,
          default=None, help='Number of jobs to run at once (passed to make)')
    parser.add_argument('-k', '--keep-outputs', action='store_true',
          default=False, help='Keep all build output files (e.g. binaries)')
    parser.add_argument('-K', '--show-config', action='store_true',
          default=False,
          help='Show configuration changes in summary (both board config files and Kconfig)')
    parser.add_argument('--preserve-config-y', action='store_true',
          default=False, help="Don't convert y to 1 in configs")
    parser.add_argument('-l', '--list-error-boards', action='store_true',
          default=False, help='Show a list of boards next to each error/warning')
    parser.add_argument('-L', '--no-lto', action='store_true',
          default=False, help='Disable Link-time Optimisation (LTO) for builds')
    parser.add_argument('--list-tool-chains', action='store_true', default=False,
          help='List available tool chains (use -v to see probing detail)')
    parser.add_argument('-m', '--mrproper', action='store_true',
          default=False, help="Run 'make mrproper before reconfiguring")
    parser.add_argument(
          '-M', '--allow-missing', action='store_true', default=False,
          help='Tell binman to allow missing blobs and generate fake ones as needed')
    parser.add_argument(
          '--maintainer-check', action='store_true',
          help='Check that maintainer entries exist for each board')
    parser.add_argument(
          '--no-allow-missing', action='store_true', default=False,
          help='Disable telling binman to allow missing blobs')
    parser.add_argument('-n', '--dry-run', action='store_true', dest='dry_run',
          default=False, help="Do a dry run (describe actions, but do nothing)")
    parser.add_argument('-N', '--no-subdirs', action='store_true', dest='no_subdirs',
          default=False,
          help="Don't create subdirectories when building current source for a single board")


def add_after_m(parser):
    """Add arguments after 'M'

    Args:
        parser (ArgumentParser): Parse to add to

    This is split out to avoid having too many statements in one function
    """
    parser.add_argument('-o', '--output-dir', type=str, dest='output_dir',
          help='Directory where all builds happen and buildman has its workspace (default is ../)')
    parser.add_argument('-O', '--override-toolchain', type=str,
          help="Override host toochain to use for sandbox (e.g. 'clang-7')")
    parser.add_argument('-Q', '--quick', action='store_true',
          default=False, help='Do a rough build, with limited warning resolution')
    parser.add_argument('-p', '--full-path', action='store_true',
          default=False, help="Use full toolchain path in CROSS_COMPILE")
    parser.add_argument('-P', '--per-board-out-dir', action='store_true',
          default=False, help="Use an O= (output) directory per board rather than per thread")
    parser.add_argument('--print-arch', action='store_true',
          default=False, help="Print the architecture for a board (ARCH=)")
    parser.add_argument('-r', '--reproducible-builds', action='store_true',
          help='Set SOURCE_DATE_EPOCH=0 to suuport a reproducible build')
    parser.add_argument('-R', '--regen-board-list', type=str,
          help='Force regeneration of the list of boards, like the old boards.cfg file')
    parser.add_argument('-s', '--summary', action='store_true',
          default=False, help='Show a build summary')
    parser.add_argument('-S', '--show-sizes', action='store_true',
          default=False, help='Show image size variation in summary')
    parser.add_argument('--step', type=int,
          default=1, help='Only build every n commits (0=just first and last)')
    if HAS_TESTS:
        parser.add_argument('--skip-net-tests', action='store_true', default=False,
                          help='Skip tests which need the network')
        parser.add_argument('-t', '--test', action='store_true', dest='test',
                          default=False, help='run tests')
        parser.add_argument('--coverage', action='store_true',
                            help='Calculated test coverage')
    parser.add_argument('-T', '--threads', type=int,
          default=None,
          help='Number of builder threads to use (0=single-thread)')
    parser.add_argument('-u', '--show_unknown', action='store_true',
          default=False, help='Show boards with unknown build result')
    parser.add_argument('-U', '--show-environment', action='store_true',
          default=False, help='Show environment changes in summary')
    parser.add_argument('-v', '--verbose', action='store_true',
          default=False, help='Show build results while the build progresses')
    parser.add_argument('-V', '--verbose-build', action='store_true',
          default=False, help='Run make with V=1, logging all output')
    parser.add_argument('-w', '--work-in-output', action='store_true',
          default=False, help='Use the output directory as the work directory')
    parser.add_argument('-W', '--ignore-warnings', action='store_true',
          default=False, help='Return success even if there are warnings')
    parser.add_argument('-x', '--exclude', dest='exclude',
          type=str, action='append',
          help='Specify a list of boards to exclude, separated by comma')
    parser.add_argument('-y', '--filter-dtb-warnings', action='store_true',
          default=False,
          help='Filter out device-tree-compiler warnings from output')
    parser.add_argument('-Y', '--filter-migration-warnings', action='store_true',
          default=False,
          help='Filter out migration warnings from output')


def parse_args():
    """Parse command line arguments from sys.argv[]

    Returns:
        tuple containing:
            options: command line options
            args: command lin arguments
    """
    epilog = """ [list of target/arch/cpu/board/vendor/soc to build]

    Build U-Boot for all commits in a branch. Use -n to do a dry run"""

    parser = argparse.ArgumentParser(epilog=epilog)
    add_upto_m(parser)
    add_after_m(parser)
    parser.add_argument('terms', type=str, nargs='*',
                        help='Board / SoC names to build')

    return parser.parse_args()
