# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2013 The Chromium OS Authors.
#

import multiprocessing
import os
import shutil
import sys

import board
import bsettings
from builder import Builder
import gitutil
import patchstream
import terminal
from terminal import Print
import toolchain
import command
import subprocess

def GetPlural(count):
    """Returns a plural 's' if count is not 1"""
    return 's' if count != 1 else ''

def GetActionSummary(is_summary, commits, selected, options):
    """Return a string summarising the intended action.

    Returns:
        Summary string.
    """
    if commits:
        count = len(commits)
        count = (count + options.step - 1) / options.step
        commit_str = '%d commit%s' % (count, GetPlural(count))
    else:
        commit_str = 'current source'
    str = '%s %s for %d boards' % (
        'Summary of' if is_summary else 'Building', commit_str,
        len(selected))
    str += ' (%d thread%s, %d job%s per thread)' % (options.threads,
            GetPlural(options.threads), options.jobs, GetPlural(options.jobs))
    return str

def ShowActions(series, why_selected, boards_selected, builder, options,
                board_warnings):
    """Display a list of actions that we would take, if not a dry run.

    Args:
        series: Series object
        why_selected: Dictionary where each key is a buildman argument
                provided by the user, and the value is the list of boards
                brought in by that argument. For example, 'arm' might bring
                in 400 boards, so in this case the key would be 'arm' and
                the value would be a list of board names.
        boards_selected: Dict of selected boards, key is target name,
                value is Board object
        builder: The builder that will be used to build the commits
        options: Command line options object
        board_warnings: List of warnings obtained from board selected
    """
    col = terminal.Color()
    print 'Dry run, so not doing much. But I would do this:'
    print
    if series:
        commits = series.commits
    else:
        commits = None
    print GetActionSummary(False, commits, boards_selected,
            options)
    print 'Build directory: %s' % builder.base_dir
    if commits:
        for upto in range(0, len(series.commits), options.step):
            commit = series.commits[upto]
            print '   ', col.Color(col.YELLOW, commit.hash[:8], bright=False),
            print commit.subject
    print
    for arg in why_selected:
        if arg != 'all':
            print arg, ': %d boards' % len(why_selected[arg])
            if options.verbose:
                print '   %s' % ' '.join(why_selected[arg])
    print ('Total boards to build for each commit: %d\n' %
            len(why_selected['all']))
    if board_warnings:
        for warning in board_warnings:
            print col.Color(col.YELLOW, warning)

def CheckOutputDir(output_dir):
    """Make sure that the output directory is not within the current directory

    If we try to use an output directory which is within the current directory
    (which is assumed to hold the U-Boot source) we may end up deleting the
    U-Boot source code. Detect this and print an error in this case.

    Args:
        output_dir: Output directory path to check
    """
    path = os.path.realpath(output_dir)
    cwd_path = os.path.realpath('.')
    while True:
        if os.path.realpath(path) == cwd_path:
            Print("Cannot use output directory '%s' since it is within the current directory '%s'" %
                  (path, cwd_path))
            sys.exit(1)
        parent = os.path.dirname(path)
        if parent == path:
            break
        path = parent

def DoBuildman(options, args, toolchains=None, make_func=None, boards=None,
               clean_dir=False):
    """The main control code for buildman

    Args:
        options: Command line options object
        args: Command line arguments (list of strings)
        toolchains: Toolchains to use - this should be a Toolchains()
                object. If None, then it will be created and scanned
        make_func: Make function to use for the builder. This is called
                to execute 'make'. If this is None, the normal function
                will be used, which calls the 'make' tool with suitable
                arguments. This setting is useful for tests.
        board: Boards() object to use, containing a list of available
                boards. If this is None it will be created and scanned.
    """
    global builder

    if options.full_help:
        pager = os.getenv('PAGER')
        if not pager:
            pager = 'more'
        fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                             'README')
        command.Run(pager, fname)
        return 0

    gitutil.Setup()
    col = terminal.Color()

    options.git_dir = os.path.join(options.git, '.git')

    no_toolchains = toolchains is None
    if no_toolchains:
        toolchains = toolchain.Toolchains(options.override_toolchain)

    if options.fetch_arch:
        if options.fetch_arch == 'list':
            sorted_list = toolchains.ListArchs()
            print col.Color(col.BLUE, 'Available architectures: %s\n' %
                            ' '.join(sorted_list))
            return 0
        else:
            fetch_arch = options.fetch_arch
            if fetch_arch == 'all':
                fetch_arch = ','.join(toolchains.ListArchs())
                print col.Color(col.CYAN, '\nDownloading toolchains: %s' %
                                fetch_arch)
            for arch in fetch_arch.split(','):
                print
                ret = toolchains.FetchAndInstall(arch)
                if ret:
                    return ret
            return 0

    if no_toolchains:
        toolchains.GetSettings()
        toolchains.Scan(options.list_tool_chains and options.verbose)
    if options.list_tool_chains:
        toolchains.List()
        print
        return 0

    # Work out how many commits to build. We want to build everything on the
    # branch. We also build the upstream commit as a control so we can see
    # problems introduced by the first commit on the branch.
    count = options.count
    has_range = options.branch and '..' in options.branch
    if count == -1:
        if not options.branch:
            count = 1
        else:
            if has_range:
                count, msg = gitutil.CountCommitsInRange(options.git_dir,
                                                         options.branch)
            else:
                count, msg = gitutil.CountCommitsInBranch(options.git_dir,
                                                          options.branch)
            if count is None:
                sys.exit(col.Color(col.RED, msg))
            elif count == 0:
                sys.exit(col.Color(col.RED, "Range '%s' has no commits" %
                                   options.branch))
            if msg:
                print col.Color(col.YELLOW, msg)
            count += 1   # Build upstream commit also

    if not count:
        str = ("No commits found to process in branch '%s': "
               "set branch's upstream or use -c flag" % options.branch)
        sys.exit(col.Color(col.RED, str))

    # Work out what subset of the boards we are building
    if not boards:
        board_file = os.path.join(options.git, 'boards.cfg')
        status = subprocess.call([os.path.join(options.git,
                                                'tools/genboardscfg.py')])
        if status != 0:
                sys.exit("Failed to generate boards.cfg")

        boards = board.Boards()
        boards.ReadBoards(os.path.join(options.git, 'boards.cfg'))

    exclude = []
    if options.exclude:
        for arg in options.exclude:
            exclude += arg.split(',')


    if options.boards:
        requested_boards = []
        for b in options.boards:
            requested_boards += b.split(',')
    else:
        requested_boards = None
    why_selected, board_warnings = boards.SelectBoards(args, exclude,
                                                       requested_boards)
    selected = boards.GetSelected()
    if not len(selected):
        sys.exit(col.Color(col.RED, 'No matching boards found'))

    # Read the metadata from the commits. First look at the upstream commit,
    # then the ones in the branch. We would like to do something like
    # upstream/master~..branch but that isn't possible if upstream/master is
    # a merge commit (it will list all the commits that form part of the
    # merge)
    # Conflicting tags are not a problem for buildman, since it does not use
    # them. For example, Series-version is not useful for buildman. On the
    # other hand conflicting tags will cause an error. So allow later tags
    # to overwrite earlier ones by setting allow_overwrite=True
    if options.branch:
        if count == -1:
            if has_range:
                range_expr = options.branch
            else:
                range_expr = gitutil.GetRangeInBranch(options.git_dir,
                                                      options.branch)
            upstream_commit = gitutil.GetUpstream(options.git_dir,
                                                  options.branch)
            series = patchstream.GetMetaDataForList(upstream_commit,
                options.git_dir, 1, series=None, allow_overwrite=True)

            series = patchstream.GetMetaDataForList(range_expr,
                    options.git_dir, None, series, allow_overwrite=True)
        else:
            # Honour the count
            series = patchstream.GetMetaDataForList(options.branch,
                    options.git_dir, count, series=None, allow_overwrite=True)
    else:
        series = None
        if not options.dry_run:
            options.verbose = True
            if not options.summary:
                options.show_errors = True

    # By default we have one thread per CPU. But if there are not enough jobs
    # we can have fewer threads and use a high '-j' value for make.
    if not options.threads:
        options.threads = min(multiprocessing.cpu_count(), len(selected))
    if not options.jobs:
        options.jobs = max(1, (multiprocessing.cpu_count() +
                len(selected) - 1) / len(selected))

    if not options.step:
        options.step = len(series.commits) - 1

    gnu_make = command.Output(os.path.join(options.git,
            'scripts/show-gnu-make'), raise_on_error=False).rstrip()
    if not gnu_make:
        sys.exit('GNU Make not found')

    # Create a new builder with the selected options.
    output_dir = options.output_dir
    if options.branch:
        dirname = options.branch.replace('/', '_')
        # As a special case allow the board directory to be placed in the
        # output directory itself rather than any subdirectory.
        if not options.no_subdirs:
            output_dir = os.path.join(options.output_dir, dirname)
        if clean_dir and os.path.exists(output_dir):
            shutil.rmtree(output_dir)
    CheckOutputDir(output_dir)
    builder = Builder(toolchains, output_dir, options.git_dir,
            options.threads, options.jobs, gnu_make=gnu_make, checkout=True,
            show_unknown=options.show_unknown, step=options.step,
            no_subdirs=options.no_subdirs, full_path=options.full_path,
            verbose_build=options.verbose_build,
            incremental=options.incremental,
            per_board_out_dir=options.per_board_out_dir,
            config_only=options.config_only,
            squash_config_y=not options.preserve_config_y,
            warnings_as_errors=options.warnings_as_errors)
    builder.force_config_on_failure = not options.quick
    if make_func:
        builder.do_make = make_func

    # For a dry run, just show our actions as a sanity check
    if options.dry_run:
        ShowActions(series, why_selected, selected, builder, options,
                    board_warnings)
    else:
        builder.force_build = options.force_build
        builder.force_build_failures = options.force_build_failures
        builder.force_reconfig = options.force_reconfig
        builder.in_tree = options.in_tree

        # Work out which boards to build
        board_selected = boards.GetSelectedDict()

        if series:
            commits = series.commits
            # Number the commits for test purposes
            for commit in range(len(commits)):
                commits[commit].sequence = commit
        else:
            commits = None

        Print(GetActionSummary(options.summary, commits, board_selected,
                                options))

        # We can't show function sizes without board details at present
        if options.show_bloat:
            options.show_detail = True
        builder.SetDisplayOptions(options.show_errors, options.show_sizes,
                                  options.show_detail, options.show_bloat,
                                  options.list_error_boards,
                                  options.show_config,
                                  options.show_environment)
        if options.summary:
            builder.ShowSummary(commits, board_selected)
        else:
            fail, warned = builder.BuildBoards(commits, board_selected,
                                options.keep_outputs, options.verbose)
            if fail:
                return 128
            elif warned:
                return 129
    return 0
