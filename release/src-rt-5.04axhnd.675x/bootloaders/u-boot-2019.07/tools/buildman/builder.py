# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2013 The Chromium OS Authors.
#
# Bloat-o-meter code used here Copyright 2004 Matt Mackall <mpm@selenic.com>
#

import collections
from datetime import datetime, timedelta
import glob
import os
import re
import Queue
import shutil
import signal
import string
import sys
import threading
import time

import builderthread
import command
import gitutil
import terminal
from terminal import Print
import toolchain


"""
Theory of Operation

Please see README for user documentation, and you should be familiar with
that before trying to make sense of this.

Buildman works by keeping the machine as busy as possible, building different
commits for different boards on multiple CPUs at once.

The source repo (self.git_dir) contains all the commits to be built. Each
thread works on a single board at a time. It checks out the first commit,
configures it for that board, then builds it. Then it checks out the next
commit and builds it (typically without re-configuring). When it runs out
of commits, it gets another job from the builder and starts again with that
board.

Clearly the builder threads could work either way - they could check out a
commit and then built it for all boards. Using separate directories for each
commit/board pair they could leave their build product around afterwards
also.

The intent behind building a single board for multiple commits, is to make
use of incremental builds. Since each commit is built incrementally from
the previous one, builds are faster. Reconfiguring for a different board
removes all intermediate object files.

Many threads can be working at once, but each has its own working directory.
When a thread finishes a build, it puts the output files into a result
directory.

The base directory used by buildman is normally '../<branch>', i.e.
a directory higher than the source repository and named after the branch
being built.

Within the base directory, we have one subdirectory for each commit. Within
that is one subdirectory for each board. Within that is the build output for
that commit/board combination.

Buildman also create working directories for each thread, in a .bm-work/
subdirectory in the base dir.

As an example, say we are building branch 'us-net' for boards 'sandbox' and
'seaboard', and say that us-net has two commits. We will have directories
like this:

us-net/             base directory
    01_of_02_g4ed4ebc_net--Add-tftp-speed-/
        sandbox/
            u-boot.bin
        seaboard/
            u-boot.bin
    02_of_02_g4ed4ebc_net--Check-tftp-comp/
        sandbox/
            u-boot.bin
        seaboard/
            u-boot.bin
    .bm-work/
        00/         working directory for thread 0 (contains source checkout)
            build/  build output
        01/         working directory for thread 1
            build/  build output
        ...
u-boot/             source directory
    .git/           repository
"""

# Possible build outcomes
OUTCOME_OK, OUTCOME_WARNING, OUTCOME_ERROR, OUTCOME_UNKNOWN = range(4)

# Translate a commit subject into a valid filename (and handle unicode)
trans_valid_chars = string.maketrans('/: ', '---')
trans_valid_chars = trans_valid_chars.decode('latin-1')

BASE_CONFIG_FILENAMES = [
    'u-boot.cfg', 'u-boot-spl.cfg', 'u-boot-tpl.cfg'
]

EXTRA_CONFIG_FILENAMES = [
    '.config', '.config-spl', '.config-tpl',
    'autoconf.mk', 'autoconf-spl.mk', 'autoconf-tpl.mk',
    'autoconf.h', 'autoconf-spl.h','autoconf-tpl.h',
]

class Config:
    """Holds information about configuration settings for a board."""
    def __init__(self, config_filename, target):
        self.target = target
        self.config = {}
        for fname in config_filename:
            self.config[fname] = {}

    def Add(self, fname, key, value):
        self.config[fname][key] = value

    def __hash__(self):
        val = 0
        for fname in self.config:
            for key, value in self.config[fname].iteritems():
                print key, value
                val = val ^ hash(key) & hash(value)
        return val

class Environment:
    """Holds information about environment variables for a board."""
    def __init__(self, target):
        self.target = target
        self.environment = {}

    def Add(self, key, value):
        self.environment[key] = value

class Builder:
    """Class for building U-Boot for a particular commit.

    Public members: (many should ->private)
        already_done: Number of builds already completed
        base_dir: Base directory to use for builder
        checkout: True to check out source, False to skip that step.
            This is used for testing.
        col: terminal.Color() object
        count: Number of commits to build
        do_make: Method to call to invoke Make
        fail: Number of builds that failed due to error
        force_build: Force building even if a build already exists
        force_config_on_failure: If a commit fails for a board, disable
            incremental building for the next commit we build for that
            board, so that we will see all warnings/errors again.
        force_build_failures: If a previously-built build (i.e. built on
            a previous run of buildman) is marked as failed, rebuild it.
        git_dir: Git directory containing source repository
        last_line_len: Length of the last line we printed (used for erasing
            it with new progress information)
        num_jobs: Number of jobs to run at once (passed to make as -j)
        num_threads: Number of builder threads to run
        out_queue: Queue of results to process
        re_make_err: Compiled regular expression for ignore_lines
        queue: Queue of jobs to run
        threads: List of active threads
        toolchains: Toolchains object to use for building
        upto: Current commit number we are building (0.count-1)
        warned: Number of builds that produced at least one warning
        force_reconfig: Reconfigure U-Boot on each comiit. This disables
            incremental building, where buildman reconfigures on the first
            commit for a baord, and then just does an incremental build for
            the following commits. In fact buildman will reconfigure and
            retry for any failing commits, so generally the only effect of
            this option is to slow things down.
        in_tree: Build U-Boot in-tree instead of specifying an output
            directory separate from the source code. This option is really
            only useful for testing in-tree builds.

    Private members:
        _base_board_dict: Last-summarised Dict of boards
        _base_err_lines: Last-summarised list of errors
        _base_warn_lines: Last-summarised list of warnings
        _build_period_us: Time taken for a single build (float object).
        _complete_delay: Expected delay until completion (timedelta)
        _next_delay_update: Next time we plan to display a progress update
                (datatime)
        _show_unknown: Show unknown boards (those not built) in summary
        _timestamps: List of timestamps for the completion of the last
            last _timestamp_count builds. Each is a datetime object.
        _timestamp_count: Number of timestamps to keep in our list.
        _working_dir: Base working directory containing all threads
    """
    class Outcome:
        """Records a build outcome for a single make invocation

        Public Members:
            rc: Outcome value (OUTCOME_...)
            err_lines: List of error lines or [] if none
            sizes: Dictionary of image size information, keyed by filename
                - Each value is itself a dictionary containing
                    values for 'text', 'data' and 'bss', being the integer
                    size in bytes of each section.
            func_sizes: Dictionary keyed by filename - e.g. 'u-boot'. Each
                    value is itself a dictionary:
                        key: function name
                        value: Size of function in bytes
            config: Dictionary keyed by filename - e.g. '.config'. Each
                    value is itself a dictionary:
                        key: config name
                        value: config value
            environment: Dictionary keyed by environment variable, Each
                     value is the value of environment variable.
        """
        def __init__(self, rc, err_lines, sizes, func_sizes, config,
                     environment):
            self.rc = rc
            self.err_lines = err_lines
            self.sizes = sizes
            self.func_sizes = func_sizes
            self.config = config
            self.environment = environment

    def __init__(self, toolchains, base_dir, git_dir, num_threads, num_jobs,
                 gnu_make='make', checkout=True, show_unknown=True, step=1,
                 no_subdirs=False, full_path=False, verbose_build=False,
                 incremental=False, per_board_out_dir=False,
                 config_only=False, squash_config_y=False,
                 warnings_as_errors=False):
        """Create a new Builder object

        Args:
            toolchains: Toolchains object to use for building
            base_dir: Base directory to use for builder
            git_dir: Git directory containing source repository
            num_threads: Number of builder threads to run
            num_jobs: Number of jobs to run at once (passed to make as -j)
            gnu_make: the command name of GNU Make.
            checkout: True to check out source, False to skip that step.
                This is used for testing.
            show_unknown: Show unknown boards (those not built) in summary
            step: 1 to process every commit, n to process every nth commit
            no_subdirs: Don't create subdirectories when building current
                source for a single board
            full_path: Return the full path in CROSS_COMPILE and don't set
                PATH
            verbose_build: Run build with V=1 and don't use 'make -s'
            incremental: Always perform incremental builds; don't run make
                mrproper when configuring
            per_board_out_dir: Build in a separate persistent directory per
                board rather than a thread-specific directory
            config_only: Only configure each build, don't build it
            squash_config_y: Convert CONFIG options with the value 'y' to '1'
            warnings_as_errors: Treat all compiler warnings as errors
        """
        self.toolchains = toolchains
        self.base_dir = base_dir
        self._working_dir = os.path.join(base_dir, '.bm-work')
        self.threads = []
        self.do_make = self.Make
        self.gnu_make = gnu_make
        self.checkout = checkout
        self.num_threads = num_threads
        self.num_jobs = num_jobs
        self.already_done = 0
        self.force_build = False
        self.git_dir = git_dir
        self._show_unknown = show_unknown
        self._timestamp_count = 10
        self._build_period_us = None
        self._complete_delay = None
        self._next_delay_update = datetime.now()
        self.force_config_on_failure = True
        self.force_build_failures = False
        self.force_reconfig = False
        self._step = step
        self.in_tree = False
        self._error_lines = 0
        self.no_subdirs = no_subdirs
        self.full_path = full_path
        self.verbose_build = verbose_build
        self.config_only = config_only
        self.squash_config_y = squash_config_y
        self.config_filenames = BASE_CONFIG_FILENAMES
        if not self.squash_config_y:
            self.config_filenames += EXTRA_CONFIG_FILENAMES

        self.warnings_as_errors = warnings_as_errors
        self.col = terminal.Color()

        self._re_function = re.compile('(.*): In function.*')
        self._re_files = re.compile('In file included from.*')
        self._re_warning = re.compile('(.*):(\d*):(\d*): warning: .*')
        self._re_dtb_warning = re.compile('(.*): Warning .*')
        self._re_note = re.compile('(.*):(\d*):(\d*): note: this is the location of the previous.*')

        self.queue = Queue.Queue()
        self.out_queue = Queue.Queue()
        for i in range(self.num_threads):
            t = builderthread.BuilderThread(self, i, incremental,
                    per_board_out_dir)
            t.setDaemon(True)
            t.start()
            self.threads.append(t)

        self.last_line_len = 0
        t = builderthread.ResultThread(self)
        t.setDaemon(True)
        t.start()
        self.threads.append(t)

        ignore_lines = ['(make.*Waiting for unfinished)', '(Segmentation fault)']
        self.re_make_err = re.compile('|'.join(ignore_lines))

        # Handle existing graceful with SIGINT / Ctrl-C
        signal.signal(signal.SIGINT, self.signal_handler)

    def __del__(self):
        """Get rid of all threads created by the builder"""
        for t in self.threads:
            del t

    def signal_handler(self, signal, frame):
        sys.exit(1)

    def SetDisplayOptions(self, show_errors=False, show_sizes=False,
                          show_detail=False, show_bloat=False,
                          list_error_boards=False, show_config=False,
                          show_environment=False):
        """Setup display options for the builder.

        show_errors: True to show summarised error/warning info
        show_sizes: Show size deltas
        show_detail: Show detail for each board
        show_bloat: Show detail for each function
        list_error_boards: Show the boards which caused each error/warning
        show_config: Show config deltas
        show_environment: Show environment deltas
        """
        self._show_errors = show_errors
        self._show_sizes = show_sizes
        self._show_detail = show_detail
        self._show_bloat = show_bloat
        self._list_error_boards = list_error_boards
        self._show_config = show_config
        self._show_environment = show_environment

    def _AddTimestamp(self):
        """Add a new timestamp to the list and record the build period.

        The build period is the length of time taken to perform a single
        build (one board, one commit).
        """
        now = datetime.now()
        self._timestamps.append(now)
        count = len(self._timestamps)
        delta = self._timestamps[-1] - self._timestamps[0]
        seconds = delta.total_seconds()

        # If we have enough data, estimate build period (time taken for a
        # single build) and therefore completion time.
        if count > 1 and self._next_delay_update < now:
            self._next_delay_update = now + timedelta(seconds=2)
            if seconds > 0:
                self._build_period = float(seconds) / count
                todo = self.count - self.upto
                self._complete_delay = timedelta(microseconds=
                        self._build_period * todo * 1000000)
                # Round it
                self._complete_delay -= timedelta(
                        microseconds=self._complete_delay.microseconds)

        if seconds > 60:
            self._timestamps.popleft()
            count -= 1

    def ClearLine(self, length):
        """Clear any characters on the current line

        Make way for a new line of length 'length', by outputting enough
        spaces to clear out the old line. Then remember the new length for
        next time.

        Args:
            length: Length of new line, in characters
        """
        if length < self.last_line_len:
            Print(' ' * (self.last_line_len - length), newline=False)
            Print('\r', newline=False)
        self.last_line_len = length
        sys.stdout.flush()

    def SelectCommit(self, commit, checkout=True):
        """Checkout the selected commit for this build
        """
        self.commit = commit
        if checkout and self.checkout:
            gitutil.Checkout(commit.hash)

    def Make(self, commit, brd, stage, cwd, *args, **kwargs):
        """Run make

        Args:
            commit: Commit object that is being built
            brd: Board object that is being built
            stage: Stage that we are at (mrproper, config, build)
            cwd: Directory where make should be run
            args: Arguments to pass to make
            kwargs: Arguments to pass to command.RunPipe()
        """
        cmd = [self.gnu_make] + list(args)
        result = command.RunPipe([cmd], capture=True, capture_stderr=True,
                cwd=cwd, raise_on_error=False, infile='/dev/null', **kwargs)
        if self.verbose_build:
            result.stdout = '%s\n' % (' '.join(cmd)) + result.stdout
            result.combined = '%s\n' % (' '.join(cmd)) + result.combined
        return result

    def ProcessResult(self, result):
        """Process the result of a build, showing progress information

        Args:
            result: A CommandResult object, which indicates the result for
                    a single build
        """
        col = terminal.Color()
        if result:
            target = result.brd.target

            self.upto += 1
            if result.return_code != 0:
                self.fail += 1
            elif result.stderr:
                self.warned += 1
            if result.already_done:
                self.already_done += 1
            if self._verbose:
                Print('\r', newline=False)
                self.ClearLine(0)
                boards_selected = {target : result.brd}
                self.ResetResultSummary(boards_selected)
                self.ProduceResultSummary(result.commit_upto, self.commits,
                                          boards_selected)
        else:
            target = '(starting)'

        # Display separate counts for ok, warned and fail
        ok = self.upto - self.warned - self.fail
        line = '\r' + self.col.Color(self.col.GREEN, '%5d' % ok)
        line += self.col.Color(self.col.YELLOW, '%5d' % self.warned)
        line += self.col.Color(self.col.RED, '%5d' % self.fail)

        name = ' /%-5d  ' % self.count

        # Add our current completion time estimate
        self._AddTimestamp()
        if self._complete_delay:
            name += '%s  : ' % self._complete_delay
        # When building all boards for a commit, we can print a commit
        # progress message.
        if result and result.commit_upto is None:
            name += 'commit %2d/%-3d' % (self.commit_upto + 1,
                    self.commit_count)

        name += target
        Print(line + name, newline=False)
        length = 16 + len(name)
        self.ClearLine(length)

    def _GetOutputDir(self, commit_upto):
        """Get the name of the output directory for a commit number

        The output directory is typically .../<branch>/<commit>.

        Args:
            commit_upto: Commit number to use (0..self.count-1)
        """
        commit_dir = None
        if self.commits:
            commit = self.commits[commit_upto]
            subject = commit.subject.translate(trans_valid_chars)
            commit_dir = ('%02d_of_%02d_g%s_%s' % (commit_upto + 1,
                    self.commit_count, commit.hash, subject[:20]))
        elif not self.no_subdirs:
            commit_dir = 'current'
        if not commit_dir:
            return self.base_dir
        return os.path.join(self.base_dir, commit_dir)

    def GetBuildDir(self, commit_upto, target):
        """Get the name of the build directory for a commit number

        The build directory is typically .../<branch>/<commit>/<target>.

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        output_dir = self._GetOutputDir(commit_upto)
        return os.path.join(output_dir, target)

    def GetDoneFile(self, commit_upto, target):
        """Get the name of the done file for a commit number

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        return os.path.join(self.GetBuildDir(commit_upto, target), 'done')

    def GetSizesFile(self, commit_upto, target):
        """Get the name of the sizes file for a commit number

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        return os.path.join(self.GetBuildDir(commit_upto, target), 'sizes')

    def GetFuncSizesFile(self, commit_upto, target, elf_fname):
        """Get the name of the funcsizes file for a commit number and ELF file

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
            elf_fname: Filename of elf image
        """
        return os.path.join(self.GetBuildDir(commit_upto, target),
                            '%s.sizes' % elf_fname.replace('/', '-'))

    def GetObjdumpFile(self, commit_upto, target, elf_fname):
        """Get the name of the objdump file for a commit number and ELF file

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
            elf_fname: Filename of elf image
        """
        return os.path.join(self.GetBuildDir(commit_upto, target),
                            '%s.objdump' % elf_fname.replace('/', '-'))

    def GetErrFile(self, commit_upto, target):
        """Get the name of the err file for a commit number

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        output_dir = self.GetBuildDir(commit_upto, target)
        return os.path.join(output_dir, 'err')

    def FilterErrors(self, lines):
        """Filter out errors in which we have no interest

        We should probably use map().

        Args:
            lines: List of error lines, each a string
        Returns:
            New list with only interesting lines included
        """
        out_lines = []
        for line in lines:
            if not self.re_make_err.search(line):
                out_lines.append(line)
        return out_lines

    def ReadFuncSizes(self, fname, fd):
        """Read function sizes from the output of 'nm'

        Args:
            fd: File containing data to read
            fname: Filename we are reading from (just for errors)

        Returns:
            Dictionary containing size of each function in bytes, indexed by
            function name.
        """
        sym = {}
        for line in fd.readlines():
            try:
                size, type, name = line[:-1].split()
            except:
                Print("Invalid line in file '%s': '%s'" % (fname, line[:-1]))
                continue
            if type in 'tTdDbB':
                # function names begin with '.' on 64-bit powerpc
                if '.' in name[1:]:
                    name = 'static.' + name.split('.')[0]
                sym[name] = sym.get(name, 0) + int(size, 16)
        return sym

    def _ProcessConfig(self, fname):
        """Read in a .config, autoconf.mk or autoconf.h file

        This function handles all config file types. It ignores comments and
        any #defines which don't start with CONFIG_.

        Args:
            fname: Filename to read

        Returns:
            Dictionary:
                key: Config name (e.g. CONFIG_DM)
                value: Config value (e.g. 1)
        """
        config = {}
        if os.path.exists(fname):
            with open(fname) as fd:
                for line in fd:
                    line = line.strip()
                    if line.startswith('#define'):
                        values = line[8:].split(' ', 1)
                        if len(values) > 1:
                            key, value = values
                        else:
                            key = values[0]
                            value = '1' if self.squash_config_y else ''
                        if not key.startswith('CONFIG_'):
                            continue
                    elif not line or line[0] in ['#', '*', '/']:
                        continue
                    else:
                        key, value = line.split('=', 1)
                    if self.squash_config_y and value == 'y':
                        value = '1'
                    config[key] = value
        return config

    def _ProcessEnvironment(self, fname):
        """Read in a uboot.env file

        This function reads in environment variables from a file.

        Args:
            fname: Filename to read

        Returns:
            Dictionary:
                key: environment variable (e.g. bootlimit)
                value: value of environment variable (e.g. 1)
        """
        environment = {}
        if os.path.exists(fname):
            with open(fname) as fd:
                for line in fd.read().split('\0'):
                    try:
                        key, value = line.split('=', 1)
                        environment[key] = value
                    except ValueError:
                        # ignore lines we can't parse
                        pass
        return environment

    def GetBuildOutcome(self, commit_upto, target, read_func_sizes,
                        read_config, read_environment):
        """Work out the outcome of a build.

        Args:
            commit_upto: Commit number to check (0..n-1)
            target: Target board to check
            read_func_sizes: True to read function size information
            read_config: True to read .config and autoconf.h files
            read_environment: True to read uboot.env files

        Returns:
            Outcome object
        """
        done_file = self.GetDoneFile(commit_upto, target)
        sizes_file = self.GetSizesFile(commit_upto, target)
        sizes = {}
        func_sizes = {}
        config = {}
        environment = {}
        if os.path.exists(done_file):
            with open(done_file, 'r') as fd:
                try:
                    return_code = int(fd.readline())
                except ValueError:
                    # The file may be empty due to running out of disk space.
                    # Try a rebuild
                    return_code = 1
                err_lines = []
                err_file = self.GetErrFile(commit_upto, target)
                if os.path.exists(err_file):
                    with open(err_file, 'r') as fd:
                        err_lines = self.FilterErrors(fd.readlines())

                # Decide whether the build was ok, failed or created warnings
                if return_code:
                    rc = OUTCOME_ERROR
                elif len(err_lines):
                    rc = OUTCOME_WARNING
                else:
                    rc = OUTCOME_OK

                # Convert size information to our simple format
                if os.path.exists(sizes_file):
                    with open(sizes_file, 'r') as fd:
                        for line in fd.readlines():
                            values = line.split()
                            rodata = 0
                            if len(values) > 6:
                                rodata = int(values[6], 16)
                            size_dict = {
                                'all' : int(values[0]) + int(values[1]) +
                                        int(values[2]),
                                'text' : int(values[0]) - rodata,
                                'data' : int(values[1]),
                                'bss' : int(values[2]),
                                'rodata' : rodata,
                            }
                            sizes[values[5]] = size_dict

            if read_func_sizes:
                pattern = self.GetFuncSizesFile(commit_upto, target, '*')
                for fname in glob.glob(pattern):
                    with open(fname, 'r') as fd:
                        dict_name = os.path.basename(fname).replace('.sizes',
                                                                    '')
                        func_sizes[dict_name] = self.ReadFuncSizes(fname, fd)

            if read_config:
                output_dir = self.GetBuildDir(commit_upto, target)
                for name in self.config_filenames:
                    fname = os.path.join(output_dir, name)
                    config[name] = self._ProcessConfig(fname)

            if read_environment:
                output_dir = self.GetBuildDir(commit_upto, target)
                fname = os.path.join(output_dir, 'uboot.env')
                environment = self._ProcessEnvironment(fname)

            return Builder.Outcome(rc, err_lines, sizes, func_sizes, config,
                                   environment)

        return Builder.Outcome(OUTCOME_UNKNOWN, [], {}, {}, {}, {})

    def GetResultSummary(self, boards_selected, commit_upto, read_func_sizes,
                         read_config, read_environment):
        """Calculate a summary of the results of building a commit.

        Args:
            board_selected: Dict containing boards to summarise
            commit_upto: Commit number to summarize (0..self.count-1)
            read_func_sizes: True to read function size information
            read_config: True to read .config and autoconf.h files
            read_environment: True to read uboot.env files

        Returns:
            Tuple:
                Dict containing boards which passed building this commit.
                    keyed by board.target
                List containing a summary of error lines
                Dict keyed by error line, containing a list of the Board
                    objects with that error
                List containing a summary of warning lines
                Dict keyed by error line, containing a list of the Board
                    objects with that warning
                Dictionary keyed by board.target. Each value is a dictionary:
                    key: filename - e.g. '.config'
                    value is itself a dictionary:
                        key: config name
                        value: config value
                Dictionary keyed by board.target. Each value is a dictionary:
                    key: environment variable
                    value: value of environment variable
        """
        def AddLine(lines_summary, lines_boards, line, board):
            line = line.rstrip()
            if line in lines_boards:
                lines_boards[line].append(board)
            else:
                lines_boards[line] = [board]
                lines_summary.append(line)

        board_dict = {}
        err_lines_summary = []
        err_lines_boards = {}
        warn_lines_summary = []
        warn_lines_boards = {}
        config = {}
        environment = {}

        for board in boards_selected.itervalues():
            outcome = self.GetBuildOutcome(commit_upto, board.target,
                                           read_func_sizes, read_config,
                                           read_environment)
            board_dict[board.target] = outcome
            last_func = None
            last_was_warning = False
            for line in outcome.err_lines:
                if line:
                    if (self._re_function.match(line) or
                            self._re_files.match(line)):
                        last_func = line
                    else:
                        is_warning = (self._re_warning.match(line) or
                                      self._re_dtb_warning.match(line))
                        is_note = self._re_note.match(line)
                        if is_warning or (last_was_warning and is_note):
                            if last_func:
                                AddLine(warn_lines_summary, warn_lines_boards,
                                        last_func, board)
                            AddLine(warn_lines_summary, warn_lines_boards,
                                    line, board)
                        else:
                            if last_func:
                                AddLine(err_lines_summary, err_lines_boards,
                                        last_func, board)
                            AddLine(err_lines_summary, err_lines_boards,
                                    line, board)
                        last_was_warning = is_warning
                        last_func = None
            tconfig = Config(self.config_filenames, board.target)
            for fname in self.config_filenames:
                if outcome.config:
                    for key, value in outcome.config[fname].iteritems():
                        tconfig.Add(fname, key, value)
            config[board.target] = tconfig

            tenvironment = Environment(board.target)
            if outcome.environment:
                for key, value in outcome.environment.iteritems():
                    tenvironment.Add(key, value)
            environment[board.target] = tenvironment

        return (board_dict, err_lines_summary, err_lines_boards,
                warn_lines_summary, warn_lines_boards, config, environment)

    def AddOutcome(self, board_dict, arch_list, changes, char, color):
        """Add an output to our list of outcomes for each architecture

        This simple function adds failing boards (changes) to the
        relevant architecture string, so we can print the results out
        sorted by architecture.

        Args:
             board_dict: Dict containing all boards
             arch_list: Dict keyed by arch name. Value is a string containing
                    a list of board names which failed for that arch.
             changes: List of boards to add to arch_list
             color: terminal.Colour object
        """
        done_arch = {}
        for target in changes:
            if target in board_dict:
                arch = board_dict[target].arch
            else:
                arch = 'unknown'
            str = self.col.Color(color, ' ' + target)
            if not arch in done_arch:
                str = ' %s  %s' % (self.col.Color(color, char), str)
                done_arch[arch] = True
            if not arch in arch_list:
                arch_list[arch] = str
            else:
                arch_list[arch] += str


    def ColourNum(self, num):
        color = self.col.RED if num > 0 else self.col.GREEN
        if num == 0:
            return '0'
        return self.col.Color(color, str(num))

    def ResetResultSummary(self, board_selected):
        """Reset the results summary ready for use.

        Set up the base board list to be all those selected, and set the
        error lines to empty.

        Following this, calls to PrintResultSummary() will use this
        information to work out what has changed.

        Args:
            board_selected: Dict containing boards to summarise, keyed by
                board.target
        """
        self._base_board_dict = {}
        for board in board_selected:
            self._base_board_dict[board] = Builder.Outcome(0, [], [], {}, {},
                                                           {})
        self._base_err_lines = []
        self._base_warn_lines = []
        self._base_err_line_boards = {}
        self._base_warn_line_boards = {}
        self._base_config = None
        self._base_environment = None

    def PrintFuncSizeDetail(self, fname, old, new):
        grow, shrink, add, remove, up, down = 0, 0, 0, 0, 0, 0
        delta, common = [], {}

        for a in old:
            if a in new:
                common[a] = 1

        for name in old:
            if name not in common:
                remove += 1
                down += old[name]
                delta.append([-old[name], name])

        for name in new:
            if name not in common:
                add += 1
                up += new[name]
                delta.append([new[name], name])

        for name in common:
                diff = new.get(name, 0) - old.get(name, 0)
                if diff > 0:
                    grow, up = grow + 1, up + diff
                elif diff < 0:
                    shrink, down = shrink + 1, down - diff
                delta.append([diff, name])

        delta.sort()
        delta.reverse()

        args = [add, -remove, grow, -shrink, up, -down, up - down]
        if max(args) == 0 and min(args) == 0:
            return
        args = [self.ColourNum(x) for x in args]
        indent = ' ' * 15
        Print('%s%s: add: %s/%s, grow: %s/%s bytes: %s/%s (%s)' %
              tuple([indent, self.col.Color(self.col.YELLOW, fname)] + args))
        Print('%s  %-38s %7s %7s %+7s' % (indent, 'function', 'old', 'new',
                                         'delta'))
        for diff, name in delta:
            if diff:
                color = self.col.RED if diff > 0 else self.col.GREEN
                msg = '%s  %-38s %7s %7s %+7d' % (indent, name,
                        old.get(name, '-'), new.get(name,'-'), diff)
                Print(msg, colour=color)


    def PrintSizeDetail(self, target_list, show_bloat):
        """Show details size information for each board

        Args:
            target_list: List of targets, each a dict containing:
                    'target': Target name
                    'total_diff': Total difference in bytes across all areas
                    <part_name>: Difference for that part
            show_bloat: Show detail for each function
        """
        targets_by_diff = sorted(target_list, reverse=True,
        key=lambda x: x['_total_diff'])
        for result in targets_by_diff:
            printed_target = False
            for name in sorted(result):
                diff = result[name]
                if name.startswith('_'):
                    continue
                if diff != 0:
                    color = self.col.RED if diff > 0 else self.col.GREEN
                msg = ' %s %+d' % (name, diff)
                if not printed_target:
                    Print('%10s  %-15s:' % ('', result['_target']),
                          newline=False)
                    printed_target = True
                Print(msg, colour=color, newline=False)
            if printed_target:
                Print()
                if show_bloat:
                    target = result['_target']
                    outcome = result['_outcome']
                    base_outcome = self._base_board_dict[target]
                    for fname in outcome.func_sizes:
                        self.PrintFuncSizeDetail(fname,
                                                 base_outcome.func_sizes[fname],
                                                 outcome.func_sizes[fname])


    def PrintSizeSummary(self, board_selected, board_dict, show_detail,
                         show_bloat):
        """Print a summary of image sizes broken down by section.

        The summary takes the form of one line per architecture. The
        line contains deltas for each of the sections (+ means the section
        got bigger, - means smaller). The nunmbers are the average number
        of bytes that a board in this section increased by.

        For example:
           powerpc: (622 boards)   text -0.0
          arm: (285 boards)   text -0.0
          nds32: (3 boards)   text -8.0

        Args:
            board_selected: Dict containing boards to summarise, keyed by
                board.target
            board_dict: Dict containing boards for which we built this
                commit, keyed by board.target. The value is an Outcome object.
            show_detail: Show detail for each board
            show_bloat: Show detail for each function
        """
        arch_list = {}
        arch_count = {}

        # Calculate changes in size for different image parts
        # The previous sizes are in Board.sizes, for each board
        for target in board_dict:
            if target not in board_selected:
                continue
            base_sizes = self._base_board_dict[target].sizes
            outcome = board_dict[target]
            sizes = outcome.sizes

            # Loop through the list of images, creating a dict of size
            # changes for each image/part. We end up with something like
            # {'target' : 'snapper9g45, 'data' : 5, 'u-boot-spl:text' : -4}
            # which means that U-Boot data increased by 5 bytes and SPL
            # text decreased by 4.
            err = {'_target' : target}
            for image in sizes:
                if image in base_sizes:
                    base_image = base_sizes[image]
                    # Loop through the text, data, bss parts
                    for part in sorted(sizes[image]):
                        diff = sizes[image][part] - base_image[part]
                        col = None
                        if diff:
                            if image == 'u-boot':
                                name = part
                            else:
                                name = image + ':' + part
                            err[name] = diff
            arch = board_selected[target].arch
            if not arch in arch_count:
                arch_count[arch] = 1
            else:
                arch_count[arch] += 1
            if not sizes:
                pass    # Only add to our list when we have some stats
            elif not arch in arch_list:
                arch_list[arch] = [err]
            else:
                arch_list[arch].append(err)

        # We now have a list of image size changes sorted by arch
        # Print out a summary of these
        for arch, target_list in arch_list.iteritems():
            # Get total difference for each type
            totals = {}
            for result in target_list:
                total = 0
                for name, diff in result.iteritems():
                    if name.startswith('_'):
                        continue
                    total += diff
                    if name in totals:
                        totals[name] += diff
                    else:
                        totals[name] = diff
                result['_total_diff'] = total
                result['_outcome'] = board_dict[result['_target']]

            count = len(target_list)
            printed_arch = False
            for name in sorted(totals):
                diff = totals[name]
                if diff:
                    # Display the average difference in this name for this
                    # architecture
                    avg_diff = float(diff) / count
                    color = self.col.RED if avg_diff > 0 else self.col.GREEN
                    msg = ' %s %+1.1f' % (name, avg_diff)
                    if not printed_arch:
                        Print('%10s: (for %d/%d boards)' % (arch, count,
                              arch_count[arch]), newline=False)
                        printed_arch = True
                    Print(msg, colour=color, newline=False)

            if printed_arch:
                Print()
                if show_detail:
                    self.PrintSizeDetail(target_list, show_bloat)


    def PrintResultSummary(self, board_selected, board_dict, err_lines,
                           err_line_boards, warn_lines, warn_line_boards,
                           config, environment, show_sizes, show_detail,
                           show_bloat, show_config, show_environment):
        """Compare results with the base results and display delta.

        Only boards mentioned in board_selected will be considered. This
        function is intended to be called repeatedly with the results of
        each commit. It therefore shows a 'diff' between what it saw in
        the last call and what it sees now.

        Args:
            board_selected: Dict containing boards to summarise, keyed by
                board.target
            board_dict: Dict containing boards for which we built this
                commit, keyed by board.target. The value is an Outcome object.
            err_lines: A list of errors for this commit, or [] if there is
                none, or we don't want to print errors
            err_line_boards: Dict keyed by error line, containing a list of
                the Board objects with that error
            warn_lines: A list of warnings for this commit, or [] if there is
                none, or we don't want to print errors
            warn_line_boards: Dict keyed by warning line, containing a list of
                the Board objects with that warning
            config: Dictionary keyed by filename - e.g. '.config'. Each
                    value is itself a dictionary:
                        key: config name
                        value: config value
            environment: Dictionary keyed by environment variable, Each
                     value is the value of environment variable.
            show_sizes: Show image size deltas
            show_detail: Show detail for each board
            show_bloat: Show detail for each function
            show_config: Show config changes
            show_environment: Show environment changes
        """
        def _BoardList(line, line_boards):
            """Helper function to get a line of boards containing a line

            Args:
                line: Error line to search for
            Return:
                String containing a list of boards with that error line, or
                '' if the user has not requested such a list
            """
            if self._list_error_boards:
                names = []
                for board in line_boards[line]:
                    if not board.target in names:
                        names.append(board.target)
                names_str = '(%s) ' % ','.join(names)
            else:
                names_str = ''
            return names_str

        def _CalcErrorDelta(base_lines, base_line_boards, lines, line_boards,
                            char):
            better_lines = []
            worse_lines = []
            for line in lines:
                if line not in base_lines:
                    worse_lines.append(char + '+' +
                            _BoardList(line, line_boards) + line)
            for line in base_lines:
                if line not in lines:
                    better_lines.append(char + '-' +
                            _BoardList(line, base_line_boards) + line)
            return better_lines, worse_lines

        def _CalcConfig(delta, name, config):
            """Calculate configuration changes

            Args:
                delta: Type of the delta, e.g. '+'
                name: name of the file which changed (e.g. .config)
                config: configuration change dictionary
                    key: config name
                    value: config value
            Returns:
                String containing the configuration changes which can be
                    printed
            """
            out = ''
            for key in sorted(config.keys()):
                out += '%s=%s ' % (key, config[key])
            return '%s %s: %s' % (delta, name, out)

        def _AddConfig(lines, name, config_plus, config_minus, config_change):
            """Add changes in configuration to a list

            Args:
                lines: list to add to
                name: config file name
                config_plus: configurations added, dictionary
                    key: config name
                    value: config value
                config_minus: configurations removed, dictionary
                    key: config name
                    value: config value
                config_change: configurations changed, dictionary
                    key: config name
                    value: config value
            """
            if config_plus:
                lines.append(_CalcConfig('+', name, config_plus))
            if config_minus:
                lines.append(_CalcConfig('-', name, config_minus))
            if config_change:
                lines.append(_CalcConfig('c', name, config_change))

        def _OutputConfigInfo(lines):
            for line in lines:
                if not line:
                    continue
                if line[0] == '+':
                    col = self.col.GREEN
                elif line[0] == '-':
                    col = self.col.RED
                elif line[0] == 'c':
                    col = self.col.YELLOW
                Print('   ' + line, newline=True, colour=col)


        ok_boards = []      # List of boards fixed since last commit
        warn_boards = []    # List of boards with warnings since last commit
        err_boards = []     # List of new broken boards since last commit
        new_boards = []     # List of boards that didn't exist last time
        unknown_boards = [] # List of boards that were not built

        for target in board_dict:
            if target not in board_selected:
                continue

            # If the board was built last time, add its outcome to a list
            if target in self._base_board_dict:
                base_outcome = self._base_board_dict[target].rc
                outcome = board_dict[target]
                if outcome.rc == OUTCOME_UNKNOWN:
                    unknown_boards.append(target)
                elif outcome.rc < base_outcome:
                    if outcome.rc == OUTCOME_WARNING:
                        warn_boards.append(target)
                    else:
                        ok_boards.append(target)
                elif outcome.rc > base_outcome:
                    if outcome.rc == OUTCOME_WARNING:
                        warn_boards.append(target)
                    else:
                        err_boards.append(target)
            else:
                new_boards.append(target)

        # Get a list of errors that have appeared, and disappeared
        better_err, worse_err = _CalcErrorDelta(self._base_err_lines,
                self._base_err_line_boards, err_lines, err_line_boards, '')
        better_warn, worse_warn = _CalcErrorDelta(self._base_warn_lines,
                self._base_warn_line_boards, warn_lines, warn_line_boards, 'w')

        # Display results by arch
        if any((ok_boards, warn_boards, err_boards, unknown_boards, new_boards,
                worse_err, better_err, worse_warn, better_warn)):
            arch_list = {}
            self.AddOutcome(board_selected, arch_list, ok_boards, '',
                    self.col.GREEN)
            self.AddOutcome(board_selected, arch_list, warn_boards, 'w+',
                    self.col.YELLOW)
            self.AddOutcome(board_selected, arch_list, err_boards, '+',
                    self.col.RED)
            self.AddOutcome(board_selected, arch_list, new_boards, '*', self.col.BLUE)
            if self._show_unknown:
                self.AddOutcome(board_selected, arch_list, unknown_boards, '?',
                        self.col.MAGENTA)
            for arch, target_list in arch_list.iteritems():
                Print('%10s: %s' % (arch, target_list))
                self._error_lines += 1
            if better_err:
                Print('\n'.join(better_err), colour=self.col.GREEN)
                self._error_lines += 1
            if worse_err:
                Print('\n'.join(worse_err), colour=self.col.RED)
                self._error_lines += 1
            if better_warn:
                Print('\n'.join(better_warn), colour=self.col.CYAN)
                self._error_lines += 1
            if worse_warn:
                Print('\n'.join(worse_warn), colour=self.col.MAGENTA)
                self._error_lines += 1

        if show_sizes:
            self.PrintSizeSummary(board_selected, board_dict, show_detail,
                                  show_bloat)

        if show_environment and self._base_environment:
            lines = []

            for target in board_dict:
                if target not in board_selected:
                    continue

                tbase = self._base_environment[target]
                tenvironment = environment[target]
                environment_plus = {}
                environment_minus = {}
                environment_change = {}
                base = tbase.environment
                for key, value in tenvironment.environment.iteritems():
                    if key not in base:
                        environment_plus[key] = value
                for key, value in base.iteritems():
                    if key not in tenvironment.environment:
                        environment_minus[key] = value
                for key, value in base.iteritems():
                    new_value = tenvironment.environment.get(key)
                    if new_value and value != new_value:
                        desc = '%s -> %s' % (value, new_value)
                        environment_change[key] = desc

                _AddConfig(lines, target, environment_plus, environment_minus,
                           environment_change)

            _OutputConfigInfo(lines)

        if show_config and self._base_config:
            summary = {}
            arch_config_plus = {}
            arch_config_minus = {}
            arch_config_change = {}
            arch_list = []

            for target in board_dict:
                if target not in board_selected:
                    continue
                arch = board_selected[target].arch
                if arch not in arch_list:
                    arch_list.append(arch)

            for arch in arch_list:
                arch_config_plus[arch] = {}
                arch_config_minus[arch] = {}
                arch_config_change[arch] = {}
                for name in self.config_filenames:
                    arch_config_plus[arch][name] = {}
                    arch_config_minus[arch][name] = {}
                    arch_config_change[arch][name] = {}

            for target in board_dict:
                if target not in board_selected:
                    continue

                arch = board_selected[target].arch

                all_config_plus = {}
                all_config_minus = {}
                all_config_change = {}
                tbase = self._base_config[target]
                tconfig = config[target]
                lines = []
                for name in self.config_filenames:
                    if not tconfig.config[name]:
                        continue
                    config_plus = {}
                    config_minus = {}
                    config_change = {}
                    base = tbase.config[name]
                    for key, value in tconfig.config[name].iteritems():
                        if key not in base:
                            config_plus[key] = value
                            all_config_plus[key] = value
                    for key, value in base.iteritems():
                        if key not in tconfig.config[name]:
                            config_minus[key] = value
                            all_config_minus[key] = value
                    for key, value in base.iteritems():
                        new_value = tconfig.config.get(key)
                        if new_value and value != new_value:
                            desc = '%s -> %s' % (value, new_value)
                            config_change[key] = desc
                            all_config_change[key] = desc

                    arch_config_plus[arch][name].update(config_plus)
                    arch_config_minus[arch][name].update(config_minus)
                    arch_config_change[arch][name].update(config_change)

                    _AddConfig(lines, name, config_plus, config_minus,
                               config_change)
                _AddConfig(lines, 'all', all_config_plus, all_config_minus,
                           all_config_change)
                summary[target] = '\n'.join(lines)

            lines_by_target = {}
            for target, lines in summary.iteritems():
                if lines in lines_by_target:
                    lines_by_target[lines].append(target)
                else:
                    lines_by_target[lines] = [target]

            for arch in arch_list:
                lines = []
                all_plus = {}
                all_minus = {}
                all_change = {}
                for name in self.config_filenames:
                    all_plus.update(arch_config_plus[arch][name])
                    all_minus.update(arch_config_minus[arch][name])
                    all_change.update(arch_config_change[arch][name])
                    _AddConfig(lines, name, arch_config_plus[arch][name],
                               arch_config_minus[arch][name],
                               arch_config_change[arch][name])
                _AddConfig(lines, 'all', all_plus, all_minus, all_change)
                #arch_summary[target] = '\n'.join(lines)
                if lines:
                    Print('%s:' % arch)
                    _OutputConfigInfo(lines)

            for lines, targets in lines_by_target.iteritems():
                if not lines:
                    continue
                Print('%s :' % ' '.join(sorted(targets)))
                _OutputConfigInfo(lines.split('\n'))


        # Save our updated information for the next call to this function
        self._base_board_dict = board_dict
        self._base_err_lines = err_lines
        self._base_warn_lines = warn_lines
        self._base_err_line_boards = err_line_boards
        self._base_warn_line_boards = warn_line_boards
        self._base_config = config
        self._base_environment = environment

        # Get a list of boards that did not get built, if needed
        not_built = []
        for board in board_selected:
            if not board in board_dict:
                not_built.append(board)
        if not_built:
            Print("Boards not built (%d): %s" % (len(not_built),
                  ', '.join(not_built)))

    def ProduceResultSummary(self, commit_upto, commits, board_selected):
            (board_dict, err_lines, err_line_boards, warn_lines,
             warn_line_boards, config, environment) = self.GetResultSummary(
                    board_selected, commit_upto,
                    read_func_sizes=self._show_bloat,
                    read_config=self._show_config,
                    read_environment=self._show_environment)
            if commits:
                msg = '%02d: %s' % (commit_upto + 1,
                        commits[commit_upto].subject)
                Print(msg, colour=self.col.BLUE)
            self.PrintResultSummary(board_selected, board_dict,
                    err_lines if self._show_errors else [], err_line_boards,
                    warn_lines if self._show_errors else [], warn_line_boards,
                    config, environment, self._show_sizes, self._show_detail,
                    self._show_bloat, self._show_config, self._show_environment)

    def ShowSummary(self, commits, board_selected):
        """Show a build summary for U-Boot for a given board list.

        Reset the result summary, then repeatedly call GetResultSummary on
        each commit's results, then display the differences we see.

        Args:
            commit: Commit objects to summarise
            board_selected: Dict containing boards to summarise
        """
        self.commit_count = len(commits) if commits else 1
        self.commits = commits
        self.ResetResultSummary(board_selected)
        self._error_lines = 0

        for commit_upto in range(0, self.commit_count, self._step):
            self.ProduceResultSummary(commit_upto, commits, board_selected)
        if not self._error_lines:
            Print('(no errors to report)', colour=self.col.GREEN)


    def SetupBuild(self, board_selected, commits):
        """Set up ready to start a build.

        Args:
            board_selected: Selected boards to build
            commits: Selected commits to build
        """
        # First work out how many commits we will build
        count = (self.commit_count + self._step - 1) / self._step
        self.count = len(board_selected) * count
        self.upto = self.warned = self.fail = 0
        self._timestamps = collections.deque()

    def GetThreadDir(self, thread_num):
        """Get the directory path to the working dir for a thread.

        Args:
            thread_num: Number of thread to check.
        """
        return os.path.join(self._working_dir, '%02d' % thread_num)

    def _PrepareThread(self, thread_num, setup_git):
        """Prepare the working directory for a thread.

        This clones or fetches the repo into the thread's work directory.

        Args:
            thread_num: Thread number (0, 1, ...)
            setup_git: True to set up a git repo clone
        """
        thread_dir = self.GetThreadDir(thread_num)
        builderthread.Mkdir(thread_dir)
        git_dir = os.path.join(thread_dir, '.git')

        # Clone the repo if it doesn't already exist
        # TODO(sjg@chromium): Perhaps some git hackery to symlink instead, so
        # we have a private index but uses the origin repo's contents?
        if setup_git and self.git_dir:
            src_dir = os.path.abspath(self.git_dir)
            if os.path.exists(git_dir):
                gitutil.Fetch(git_dir, thread_dir)
            else:
                Print('\rCloning repo for thread %d' % thread_num,
                      newline=False)
                gitutil.Clone(src_dir, thread_dir)
                Print('\r%s\r' % (' ' * 30), newline=False)

    def _PrepareWorkingSpace(self, max_threads, setup_git):
        """Prepare the working directory for use.

        Set up the git repo for each thread.

        Args:
            max_threads: Maximum number of threads we expect to need.
            setup_git: True to set up a git repo clone
        """
        builderthread.Mkdir(self._working_dir)
        for thread in range(max_threads):
            self._PrepareThread(thread, setup_git)

    def _PrepareOutputSpace(self):
        """Get the output directories ready to receive files.

        We delete any output directories which look like ones we need to
        create. Having left over directories is confusing when the user wants
        to check the output manually.
        """
        if not self.commits:
            return
        dir_list = []
        for commit_upto in range(self.commit_count):
            dir_list.append(self._GetOutputDir(commit_upto))

        to_remove = []
        for dirname in glob.glob(os.path.join(self.base_dir, '*')):
            if dirname not in dir_list:
                to_remove.append(dirname)
        if to_remove:
            Print('Removing %d old build directories' % len(to_remove),
                  newline=False)
            for dirname in to_remove:
                shutil.rmtree(dirname)

    def BuildBoards(self, commits, board_selected, keep_outputs, verbose):
        """Build all commits for a list of boards

        Args:
            commits: List of commits to be build, each a Commit object
            boards_selected: Dict of selected boards, key is target name,
                    value is Board object
            keep_outputs: True to save build output files
            verbose: Display build results as they are completed
        Returns:
            Tuple containing:
                - number of boards that failed to build
                - number of boards that issued warnings
        """
        self.commit_count = len(commits) if commits else 1
        self.commits = commits
        self._verbose = verbose

        self.ResetResultSummary(board_selected)
        builderthread.Mkdir(self.base_dir, parents = True)
        self._PrepareWorkingSpace(min(self.num_threads, len(board_selected)),
                commits is not None)
        self._PrepareOutputSpace()
        Print('\rStarting build...', newline=False)
        self.SetupBuild(board_selected, commits)
        self.ProcessResult(None)

        # Create jobs to build all commits for each board
        for brd in board_selected.itervalues():
            job = builderthread.BuilderJob()
            job.board = brd
            job.commits = commits
            job.keep_outputs = keep_outputs
            job.step = self._step
            self.queue.put(job)

        term = threading.Thread(target=self.queue.join)
        term.setDaemon(True)
        term.start()
        while term.isAlive():
            term.join(100)

        # Wait until we have processed all output
        self.out_queue.join()
        Print()
        self.ClearLine(0)
        return (self.fail, self.warned)
