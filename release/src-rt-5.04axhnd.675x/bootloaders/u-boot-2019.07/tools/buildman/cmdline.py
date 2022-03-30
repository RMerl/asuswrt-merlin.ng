# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2014 Google, Inc
#

from optparse import OptionParser

def ParseArgs():
    """Parse command line arguments from sys.argv[]

    Returns:
        tuple containing:
            options: command line options
            args: command lin arguments
    """
    parser = OptionParser()
    parser.add_option('-b', '--branch', type='string',
          help='Branch name to build, or range of commits to build')
    parser.add_option('-B', '--bloat', dest='show_bloat',
          action='store_true', default=False,
          help='Show changes in function code size for each board')
    parser.add_option('--boards', type='string', action='append',
          help='List of board names to build separated by comma')
    parser.add_option('-c', '--count', dest='count', type='int',
          default=-1, help='Run build on the top n commits')
    parser.add_option('-C', '--force-reconfig', dest='force_reconfig',
          action='store_true', default=False,
          help='Reconfigure for every commit (disable incremental build)')
    parser.add_option('-d', '--detail', dest='show_detail',
          action='store_true', default=False,
          help='Show detailed information for each board in summary')
    parser.add_option('-D', '--config-only', action='store_true', default=False,
          help="Don't build, just configure each commit")
    parser.add_option('-e', '--show_errors', action='store_true',
          default=False, help='Show errors and warnings')
    parser.add_option('-E', '--warnings-as-errors', action='store_true',
          default=False, help='Treat all compiler warnings as errors')
    parser.add_option('-f', '--force-build', dest='force_build',
          action='store_true', default=False,
          help='Force build of boards even if already built')
    parser.add_option('-F', '--force-build-failures', dest='force_build_failures',
          action='store_true', default=False,
          help='Force build of previously-failed build')
    parser.add_option('--fetch-arch', type='string',
          help="Fetch a toolchain for architecture FETCH_ARCH ('list' to list)."
              ' You can also fetch several toolchains separate by comma, or'
              " 'all' to download all")
    parser.add_option('-g', '--git', type='string',
          help='Git repo containing branch to build', default='.')
    parser.add_option('-G', '--config-file', type='string',
          help='Path to buildman config file', default='')
    parser.add_option('-H', '--full-help', action='store_true', dest='full_help',
          default=False, help='Display the README file')
    parser.add_option('-i', '--in-tree', dest='in_tree',
          action='store_true', default=False,
          help='Build in the source tree instead of a separate directory')
    parser.add_option('-I', '--incremental', action='store_true',
          default=False, help='Do not run make mrproper (when reconfiguring)')
    parser.add_option('-j', '--jobs', dest='jobs', type='int',
          default=None, help='Number of jobs to run at once (passed to make)')
    parser.add_option('-k', '--keep-outputs', action='store_true',
          default=False, help='Keep all build output files (e.g. binaries)')
    parser.add_option('-K', '--show-config', action='store_true',
          default=False, help='Show configuration changes in summary (both board config files and Kconfig)')
    parser.add_option('--preserve-config-y', action='store_true',
          default=False, help="Don't convert y to 1 in configs")
    parser.add_option('-l', '--list-error-boards', action='store_true',
          default=False, help='Show a list of boards next to each error/warning')
    parser.add_option('--list-tool-chains', action='store_true', default=False,
          help='List available tool chains (use -v to see probing detail)')
    parser.add_option('-n', '--dry-run', action='store_true', dest='dry_run',
          default=False, help="Do a dry run (describe actions, but do nothing)")
    parser.add_option('-N', '--no-subdirs', action='store_true', dest='no_subdirs',
          default=False, help="Don't create subdirectories when building current source for a single board")
    parser.add_option('-o', '--output-dir', type='string',
          dest='output_dir', default='..',
          help='Directory where all builds happen and buildman has its workspace (default is ../)')
    parser.add_option('-O', '--override-toolchain', type='string',
          help="Override host toochain to use for sandbox (e.g. 'clang-7')")
    parser.add_option('-Q', '--quick', action='store_true',
          default=False, help='Do a rough build, with limited warning resolution')
    parser.add_option('-p', '--full-path', action='store_true',
          default=False, help="Use full toolchain path in CROSS_COMPILE")
    parser.add_option('-P', '--per-board-out-dir', action='store_true',
          default=False, help="Use an O= (output) directory per board rather than per thread")
    parser.add_option('-s', '--summary', action='store_true',
          default=False, help='Show a build summary')
    parser.add_option('-S', '--show-sizes', action='store_true',
          default=False, help='Show image size variation in summary')
    parser.add_option('--skip-net-tests', action='store_true', default=False,
                      help='Skip tests which need the network')
    parser.add_option('--step', type='int',
          default=1, help='Only build every n commits (0=just first and last)')
    parser.add_option('-t', '--test', action='store_true', dest='test',
                      default=False, help='run tests')
    parser.add_option('-T', '--threads', type='int',
          default=None, help='Number of builder threads to use')
    parser.add_option('-u', '--show_unknown', action='store_true',
          default=False, help='Show boards with unknown build result')
    parser.add_option('-U', '--show-environment', action='store_true',
          default=False, help='Show environment changes in summary')
    parser.add_option('-v', '--verbose', action='store_true',
          default=False, help='Show build results while the build progresses')
    parser.add_option('-V', '--verbose-build', action='store_true',
          default=False, help='Run make with V=1, logging all output')
    parser.add_option('-x', '--exclude', dest='exclude',
          type='string', action='append',
          help='Specify a list of boards to exclude, separated by comma')

    parser.usage += """ [list of target/arch/cpu/board/vendor/soc to build]

    Build U-Boot for all commits in a branch. Use -n to do a dry run"""

    return parser.parse_args()
