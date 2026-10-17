#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Author: Masahiro Yamada <yamada.masahiro@socionext.com>
#

"""
Build and query a Kconfig database for boards.

See doc/develop/moveconfig.rst for documentation.
"""

from argparse import ArgumentParser
import collections
from contextlib import ExitStack
import doctest
import filecmp
import fnmatch
import glob
import multiprocessing
import os
import queue
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest

import asteval
from buildman import bsettings
from buildman import kconfiglib
from buildman import toolchain
from u_boot_pylib import terminal

SHOW_GNU_MAKE = 'scripts/show-gnu-make'
SLEEP_TIME=0.03

STATE_IDLE = 0
STATE_DEFCONFIG = 1
STATE_AUTOCONF = 2
STATE_SAVEDEFCONFIG = 3

AUTO_CONF_PATH = 'include/config/auto.conf'
CONFIG_DATABASE = 'qconfig.db'
FAILED_LIST = 'qconfig.failed'

CONFIG_LEN = len('CONFIG_')

SIZES = {
    'SZ_1':    0x00000001, 'SZ_2':    0x00000002,
    'SZ_4':    0x00000004, 'SZ_8':    0x00000008,
    'SZ_16':   0x00000010, 'SZ_32':   0x00000020,
    'SZ_64':   0x00000040, 'SZ_128':  0x00000080,
    'SZ_256':  0x00000100, 'SZ_512':  0x00000200,
    'SZ_1K':   0x00000400, 'SZ_2K':   0x00000800,
    'SZ_4K':   0x00001000, 'SZ_8K':   0x00002000,
    'SZ_16K':  0x00004000, 'SZ_32K':  0x00008000,
    'SZ_64K':  0x00010000, 'SZ_128K': 0x00020000,
    'SZ_256K': 0x00040000, 'SZ_512K': 0x00080000,
    'SZ_1M':   0x00100000, 'SZ_2M':   0x00200000,
    'SZ_4M':   0x00400000, 'SZ_8M':   0x00800000,
    'SZ_16M':  0x01000000, 'SZ_32M':  0x02000000,
    'SZ_64M':  0x04000000, 'SZ_128M': 0x08000000,
    'SZ_256M': 0x10000000, 'SZ_512M': 0x20000000,
    'SZ_1G':   0x40000000, 'SZ_2G':   0x80000000,
    'SZ_4G':  0x100000000
}

RE_REMOVE_DEFCONFIG = re.compile(r'(.*)_defconfig')

# CONFIG symbols present in the build system (from Linux) but not actually used
# in U-Boot; KCONFIG symbols
IGNORE_SYMS = ['DEBUG_SECTION_MISMATCH', 'FTRACE_MCOUNT_RECORD', 'GCOV_KERNEL',
               'GCOV_PROFILE_ALL', 'KALLSYMS', 'KASAN', 'MODVERSIONS', 'SHELL',
               'TPL_BUILD', 'VPL_BUILD', 'IS_ENABLED', 'FOO', 'IF_ENABLED_INT',
               'IS_ENABLED_', 'IS_ENABLED_1', 'IS_ENABLED_2', 'IS_ENABLED_3',
               'SPL_', 'TPL_', 'SPL_FOO', 'TPL_FOO', 'TOOLS_FOO',
               'ACME', 'SPL_ACME', 'TPL_ACME', 'TRACE_BRANCH_PROFILING',
               'VAL', '_UNDEFINED', 'SPL_BUILD', ]

SPL_PREFIXES = ['SPL_', 'TPL_', 'VPL_', 'TOOLS_']

### helper functions ###
def check_top_directory():
    """Exit if we are not at the top of source directory."""
    for fname in 'README', 'Licenses':
        if not os.path.exists(fname):
            sys.exit('Please run at the top of source directory.')

def check_clean_directory():
    """Exit if the source tree is not clean."""
    for fname in '.config', 'include/config':
        if os.path.exists(fname):
            sys.exit("source tree is not clean, please run 'make mrproper'")

def get_make_cmd():
    """Get the command name of GNU Make.

    U-Boot needs GNU Make for building, but the command name is not
    necessarily "make". (for example, "gmake" on FreeBSD).
    Returns the most appropriate command name on your system.
    """
    with subprocess.Popen([SHOW_GNU_MAKE], stdout=subprocess.PIPE) as proc:
        ret = proc.communicate()
        if proc.returncode:
            sys.exit('GNU Make not found')
    return ret[0].rstrip()

def get_matched_defconfig(line):
    """Get the defconfig files that match a pattern

    Args:
        line (str): Path or filename to match, e.g. 'configs/snow_defconfig' or
            'k2*_defconfig'. If no directory is provided, 'configs/' is
            prepended

    Returns:
        list of str: a list of matching defconfig files
    """
    dirname = os.path.dirname(line)
    if dirname:
        pattern = line
    else:
        pattern = os.path.join('configs', line)
    return glob.glob(pattern) + glob.glob(pattern + '_defconfig')

def get_matched_defconfigs(defconfigs_file):
    """Get all the defconfig files that match the patterns in a file.

    Args:
        defconfigs_file (str): File containing a list of defconfigs to process,
            or '-' to read the list from stdin

    Returns:
        list of str: A list of paths to defconfig files, with no duplicates
    """
    defconfigs = []
    with ExitStack() as stack:
        if defconfigs_file == '-':
            inf = sys.stdin
            defconfigs_file = 'stdin'
        else:
            inf = stack.enter_context(open(defconfigs_file, encoding='utf-8'))
        for i, line in enumerate(inf):
            line = line.strip()
            if not line:
                continue # skip blank lines silently
            if ' ' in line:
                line = line.split(' ')[0]  # handle 'git log' input
            matched = get_matched_defconfig(line)
            if not matched:
                print(f"warning: {defconfigs_file}:{i + 1}: no defconfig matched '{line}'",
                      file=sys.stderr)

            defconfigs += matched

    # use set() to drop multiple matching
    return [defconfig[len('configs') + 1:]  for defconfig in set(defconfigs)]

def get_all_defconfigs():
    """Get all the defconfig files under the configs/ directory.

    Returns:
        list of str: List of paths to defconfig files
    """
    defconfigs = []
    for (dirpath, _, filenames) in os.walk('configs'):
        dirpath = dirpath[len('configs') + 1:]
        for filename in fnmatch.filter(filenames, '*_defconfig'):
            defconfigs.append(os.path.join(dirpath, filename))

    return defconfigs

def write_file(fname, data):
    """Write data to a file

    Args:
        fname (str): Filename to write to
        data (list of str): Lines to write (with or without trailing newline);
            or str to write
    """
    with open(fname, 'w', encoding='utf-8') as out:
        if isinstance(data, list):
            for line in data:
                print(line.rstrip('\n'), file=out)
        else:
            out.write(data)

def read_file(fname, as_lines=True, skip_unicode=False):
    """Read a file and return the contents

    Args:
        fname (str): Filename to read from
        as_lines (bool): Return file contents as a list of lines
        skip_unicode (bool): True to report unicode errors and continue

    Returns:
        iter of str: List of ;ines from the file with newline removed; str if
            as_lines is False with newlines intact; or None if a unicode error
            occurred

    Raises:
        UnicodeDecodeError: Unicode error occurred when reading
    """
    with open(fname, encoding='utf-8') as inf:
        try:
            if as_lines:
                return [line.rstrip('\n') for line in inf.readlines()]
            return inf.read()
        except UnicodeDecodeError as exc:
            if not skip_unicode:
                raise
            print(f"Failed on file '{fname}: {exc}")
            return None

def try_expand(line):
    """If value looks like an expression, try expanding it
    Otherwise just return the existing value
    """
    if line.find('=') == -1:
        return line

    try:
        aeval = asteval.Interpreter( usersyms=SIZES, minimal=True )
        cfg, val = re.split("=", line)
        val= val.strip('\"')
        if re.search(r'[*+-/]|<<|SZ_+|\(([^\)]+)\)', val):
            newval = hex(aeval(val))
            print(f'\tExpanded expression {val} to {newval}')
            return cfg+'='+newval
    except:
        print(f'\tFailed to expand expression in {line}')

    return line


### classes ###
class Progress:

    """Progress Indicator"""

    def __init__(self, col, total):
        """Create a new progress indicator.

        Args:
            color_enabled (bool): True for colour output
            total (int): A number of defconfig files to process.
        """
        self.col = col
        self.current = 0
        self.good = 0
        self.total = total

    def inc(self, success):
        """Increment the number of processed defconfig files.

        Args:
            success (bool): True if processing succeeded
        """
        self.good += success
        self.current += 1

    def show(self):
        """Display the progress."""
        if self.current != self.total:
            line = self.col.build(self.col.GREEN, f'{self.good:5d}')
            line += self.col.build(self.col.RED,
                                   f'{self.current - self.good:5d}')
            line += self.col.build(self.col.MAGENTA,
                                   f'/{self.total - self.current}')
            print(f'{line}  \r', end='')
        sys.stdout.flush()


class KconfigScanner:
    """Kconfig scanner."""

    def __init__(self):
        """Scan all the Kconfig files and create a Config object."""
        # Define environment variables referenced from Kconfig
        os.environ['srctree'] = os.getcwd()
        os.environ['UBOOTVERSION'] = 'dummy'
        os.environ['KCONFIG_OBJDIR'] = ''
        os.environ['CC'] = 'gcc'
        self.conf = kconfiglib.Kconfig()


class KconfigParser:

    """A parser of .config and include/autoconf.mk."""

    re_arch = re.compile(r'CONFIG_SYS_ARCH="(.*)"')
    re_cpu = re.compile(r'CONFIG_SYS_CPU="(.*)"')

    def __init__(self, args, build_dir):
        """Create a new parser.

        Args:
          args (Namespace): program arguments
          build_dir: Build directory.
        """
        self.args = args
        self.dotconfig = os.path.join(build_dir, '.config')
        self.autoconf = os.path.join(build_dir, 'include', 'autoconf.mk')
        self.spl_autoconf = os.path.join(build_dir, 'spl', 'include',
                                         'autoconf.mk')
        self.config_autoconf = os.path.join(build_dir, AUTO_CONF_PATH)
        self.defconfig = os.path.join(build_dir, 'defconfig')

    def get_arch(self):
        """Parse .config file and return the architecture.

        Returns:
          Architecture name (e.g. 'arm').
        """
        arch = ''
        cpu = ''
        for line in read_file(self.dotconfig):
            m_arch = self.re_arch.match(line)
            if m_arch:
                arch = m_arch.group(1)
                continue
            m_cpu = self.re_cpu.match(line)
            if m_cpu:
                cpu = m_cpu.group(1)

        if not arch:
            return None

        # fix-up for aarch64
        if arch == 'arm' and cpu == 'armv8':
            arch = 'aarch64'

        return arch


class DatabaseThread(threading.Thread):
    """This thread processes results from Slot threads.

    It collects the data in the master config directary. There is only one
    result thread, and this helps to serialise the build output.
    """
    def __init__(self, config_db, db_queue):
        """Set up a new result thread

        Args:
            builder: Builder which will be sent each result
        """
        threading.Thread.__init__(self)
        self.config_db = config_db
        self.db_queue= db_queue

    def run(self):
        """Called to start up the result thread.

        We collect the next result job and pass it on to the build.
        """
        while True:
            defconfig, configs = self.db_queue.get()
            self.config_db[defconfig] = configs
            self.db_queue.task_done()


class Slot:

    """A slot to store a subprocess.

    Each instance of this class handles one subprocess.
    This class is useful to control multiple threads
    for faster processing.
    """

    def __init__(self, toolchains, args, progress, devnull, make_cmd,
                 reference_src_dir, db_queue, col):
        """Create a new process slot.

        Args:
          toolchains: Toolchains object containing toolchains.
          args: Program arguments
          progress: A progress indicator.
          devnull: A file object of '/dev/null'.
          make_cmd: command name of GNU Make.
          reference_src_dir: Determine the true starting config state from this
                             source tree.
          db_queue: output queue to write config info for the database
          col (terminal.Color): Colour object
        """
        self.toolchains = toolchains
        self.args = args
        self.progress = progress
        self.build_dir = tempfile.mkdtemp()
        self.devnull = devnull
        self.make_cmd = (make_cmd, 'O=' + self.build_dir)
        self.reference_src_dir = reference_src_dir
        self.db_queue = db_queue
        self.col = col
        self.parser = KconfigParser(args, self.build_dir)
        self.state = STATE_IDLE
        self.failed_boards = set()
        self.defconfig = None
        self.log = []
        self.current_src_dir = None
        self.proc = None

    def __del__(self):
        """Delete the working directory

        This function makes sure the temporary directory is cleaned away
        even if Python suddenly dies due to error.  It should be done in here
        because it is guaranteed the destructor is always invoked when the
        instance of the class gets unreferenced.

        If the subprocess is still running, wait until it finishes.
        """
        if self.state != STATE_IDLE:
            while self.proc.poll() is None:
                pass
        shutil.rmtree(self.build_dir)

    def add(self, defconfig):
        """Assign a new subprocess for defconfig and add it to the slot.

        If the slot is vacant, create a new subprocess for processing the
        given defconfig and add it to the slot.  Just returns False if
        the slot is occupied (i.e. the current subprocess is still running).

        Args:
          defconfig (str): defconfig name.

        Returns:
          Return True on success or False on failure
        """
        if self.state != STATE_IDLE:
            return False

        self.defconfig = defconfig
        self.log = []
        self.current_src_dir = self.reference_src_dir
        self.do_defconfig()
        return True

    def poll(self):
        """Check the status of the subprocess and handle it as needed.

        Returns True if the slot is vacant (i.e. in idle state).
        If the configuration is successfully finished, assign a new
        subprocess to build include/autoconf.mk.
        If include/autoconf.mk is generated, invoke the parser to
        parse the .config and the include/autoconf.mk, moving
        config options to the .config as needed.
        If the .config was updated, run "make savedefconfig" to sync
        it, update the original defconfig, and then set the slot back
        to the idle state.

        Returns:
          Return True if the subprocess is terminated, False otherwise
        """
        if self.state == STATE_IDLE:
            return True

        if self.proc.poll() is None:
            return False

        if self.proc.poll() != 0:
            self.handle_error()
        elif self.state == STATE_DEFCONFIG:
            if self.reference_src_dir and not self.current_src_dir:
                self.do_savedefconfig()
            else:
                self.do_autoconf()
        elif self.state == STATE_AUTOCONF:
            if self.current_src_dir:
                self.current_src_dir = None
                self.do_defconfig()
            elif self.args.build_db:
                self.do_build_db()
            else:
                self.do_savedefconfig()
        elif self.state == STATE_SAVEDEFCONFIG:
            self.update_defconfig()
        else:
            sys.exit('Internal Error. This should not happen.')

        return self.state == STATE_IDLE

    def handle_error(self):
        """Handle error cases."""

        self.log.append(self.col.build(self.col.RED, 'Failed to process',
                                       bright=True))
        if self.args.verbose:
            for line in self.proc.stderr.read().decode().splitlines():
                self.log.append(self.col.build(self.col.CYAN, line, True))
        self.finish(False)

    def do_defconfig(self):
        """Run 'make <board>_defconfig' to create the .config file."""

        cmd = list(self.make_cmd)
        cmd.append(self.defconfig)
        self.proc = subprocess.Popen(cmd, stdout=self.devnull,
                                     stderr=subprocess.PIPE,
                                     cwd=self.current_src_dir)
        self.state = STATE_DEFCONFIG

    def do_autoconf(self):
        """Run 'make AUTO_CONF_PATH'."""

        arch = self.parser.get_arch()
        try:
            tchain = self.toolchains.Select(arch)
        except ValueError:
            self.log.append(self.col.build(
                self.col.YELLOW,
                f"Tool chain for '{arch}' is missing: do nothing"))
            self.finish(False)
            return
        env = tchain.MakeEnvironment(False)

        cmd = list(self.make_cmd)
        cmd.append('KCONFIG_IGNORE_DUPLICATES=1')
        cmd.append(AUTO_CONF_PATH)
        self.proc = subprocess.Popen(cmd, stdout=self.devnull, env=env,
                                     stderr=subprocess.PIPE,
                                     cwd=self.current_src_dir)
        self.state = STATE_AUTOCONF

    def do_build_db(self):
        """Add the board to the database"""
        configs = {}
        for line in read_file(os.path.join(self.build_dir, AUTO_CONF_PATH)):
            if line.startswith('CONFIG'):
                config, value = line.split('=', 1)
                configs[config] = value.rstrip()
        self.db_queue.put([self.defconfig, configs])
        self.finish(True)

    def do_savedefconfig(self):
        """Update the .config and run 'make savedefconfig'."""
        if not self.args.force_sync:
            self.finish(True)
            return

        cmd = list(self.make_cmd)
        cmd.append('savedefconfig')
        self.proc = subprocess.Popen(cmd, stdout=self.devnull,
                                     stderr=subprocess.PIPE)
        self.state = STATE_SAVEDEFCONFIG

    def update_defconfig(self):
        """Update the input defconfig and go back to the idle state."""
        orig_defconfig = os.path.join('configs', self.defconfig)
        new_defconfig = os.path.join(self.build_dir, 'defconfig')
        updated = not filecmp.cmp(orig_defconfig, new_defconfig)

        if updated:
            self.log.append(
                self.col.build(self.col.BLUE, 'defconfig updated', bright=True))

        if not self.args.dry_run and updated:
            shutil.move(new_defconfig, orig_defconfig)
        self.finish(True)

    def finish(self, success):
        """Display log along with progress and go to the idle state.

        Args:
          success (bool): Should be True when the defconfig was processed
                   successfully, or False when it fails.
        """
        # output at least 30 characters to hide the "* defconfigs out of *".
        name = self.defconfig[:-len('_defconfig')]
        if self.log:

            # Put the first log line on the first line
            log = name.ljust(20) + ' ' + self.log[0]

            if len(self.log) > 1:
                log += '\n' + '\n'.join(['    ' + s for s in self.log[1:]])
            # Some threads are running in parallel.
            # Print log atomically to not mix up logs from different threads.
            print(log, file=(sys.stdout if success else sys.stderr))

        if not success:
            if self.args.exit_on_error:
                sys.exit('Exit on error.')
            # If --exit-on-error flag is not set, skip this board and continue.
            # Record the failed board.
            self.failed_boards.add(name)

        self.progress.inc(success)
        self.progress.show()
        self.state = STATE_IDLE

    def get_failed_boards(self):
        """Returns a set of failed boards (defconfigs) in this slot.
        """
        return self.failed_boards

class Slots:

    """Controller of the array of subprocess slots."""

    def __init__(self, toolchains, args, progress, reference_src_dir, db_queue,
                 col):
        """Create a new slots controller.

        Args:
            toolchains (Toolchains): Toolchains object containing toolchains
            args (Namespace): Program arguments
            progress (Progress): A progress indicator.
            reference_src_dir (str): Determine the true starting config state
                from this source tree (None for none)
            db_queue (Queue): output queue to write config info for the database
            col (terminal.Color): Colour object
        """
        self.args = args
        self.slots = []
        self.progress = progress
        self.col = col
        devnull = subprocess.DEVNULL
        make_cmd = get_make_cmd()
        for _ in range(args.jobs):
            self.slots.append(Slot(toolchains, args, progress, devnull,
                                   make_cmd, reference_src_dir, db_queue, col))

    def add(self, defconfig):
        """Add a new subprocess if a vacant slot is found.

        Args:
          defconfig (str): defconfig name to be put into.

        Returns:
          Return True on success or False on failure
        """
        for slot in self.slots:
            if slot.add(defconfig):
                return True
        return False

    def available(self):
        """Check if there is a vacant slot.

        Returns:
          Return True if at lease one vacant slot is found, False otherwise.
        """
        for slot in self.slots:
            if slot.poll():
                return True
        return False

    def empty(self):
        """Check if all slots are vacant.

        Returns:
          Return True if all the slots are vacant, False otherwise.
        """
        ret = True
        for slot in self.slots:
            if not slot.poll():
                ret = False
        return ret

    def write_failed_boards(self):
        """Show the results of processing"""
        boards = set()

        for slot in self.slots:
            boards |= slot.get_failed_boards()

        if boards:
            boards = '\n'.join(sorted(boards)) + '\n'
            write_file(FAILED_LIST, boards)


class ReferenceSource:

    """Reference source against which original configs should be parsed."""

    def __init__(self, commit):
        """Create a reference source directory based on a specified commit.

        Args:
          commit: commit to git-clone
        """
        self.src_dir = tempfile.mkdtemp()
        print('Cloning git repo to a separate work directory...')
        subprocess.check_output(['git', 'clone', os.getcwd(), '.'],
                                cwd=self.src_dir)
        rev = subprocess.check_output(['git', 'rev-parse', '--short',
                                       commit]).strip()
        print(f"Checkout '{rev}' to build the original autoconf.mk.")
        subprocess.check_output(['git', 'checkout', commit],
                                stderr=subprocess.STDOUT, cwd=self.src_dir)

    def __del__(self):
        """Delete the reference source directory

        This function makes sure the temporary directory is cleaned away
        even if Python suddenly dies due to error.  It should be done in here
        because it is guaranteed the destructor is always invoked when the
        instance of the class gets unreferenced.
        """
        shutil.rmtree(self.src_dir)

    def get_dir(self):
        """Return the absolute path to the reference source directory."""

        return self.src_dir

def move_config(toolchains, args, db_queue, col):
    """Build database or sync config options to defconfig files.

    Args:
        toolchains (Toolchains): Toolchains to use
        args (Namespace): Program arguments
        db_queue (Queue): Queue for database updates
        col (terminal.Color): Colour object

    Returns:
        Progress: Progress indicator
    """
    if args.git_ref:
        reference_src = ReferenceSource(args.git_ref)
        reference_src_dir = reference_src.get_dir()
    else:
        reference_src_dir = None

    if args.defconfigs:
        defconfigs = get_matched_defconfigs(args.defconfigs)
    else:
        defconfigs = get_all_defconfigs()

    progress = Progress(col, len(defconfigs))
    slots = Slots(toolchains, args, progress, reference_src_dir, db_queue, col)

    # Main loop to process defconfig files:
    #  Add a new subprocess into a vacant slot.
    #  Sleep if there is no available slot.
    for defconfig in defconfigs:
        while not slots.add(defconfig):
            while not slots.available():
                # No available slot: sleep for a while
                time.sleep(SLEEP_TIME)

    # wait until all the subprocesses finish
    while not slots.empty():
        time.sleep(SLEEP_TIME)

    slots.write_failed_boards()
    return progress

def find_kconfig_rules(kconf, config, imply_config):
    """Check whether a config has a 'select' or 'imply' keyword

    Args:
        kconf (Kconfiglib.Kconfig): Kconfig object
        config (str): Name of config to check (without CONFIG_ prefix)
        imply_config (str): Implying config (without CONFIG_ prefix) which may
            or may not have an 'imply' for 'config')

    Returns:
        Symbol object for 'config' if found, else None
    """
    sym = kconf.syms.get(imply_config)
    if sym:
        for sel, _ in (sym.selects + sym.implies):
            if sel.name == config:
                return sym
    return None

def check_imply_rule(kconf, config, imply_config):
    """Check if we can add an 'imply' option

    This finds imply_config in the Kconfig and looks to see if it is possible
    to add an 'imply' for 'config' to that part of the Kconfig.

    Args:
        kconf (Kconfiglib.Kconfig): Kconfig object
        config (str): Name of config to check (without CONFIG_ prefix)
        imply_config (str): Implying config (without CONFIG_ prefix) which may
            or may not have an 'imply' for 'config')

    Returns:
        tuple:
            str: filename of Kconfig file containing imply_config, or None if
                none
            int: line number within the Kconfig file, or 0 if none
            str: message indicating the result
    """
    sym = kconf.syms.get(imply_config)
    if not sym:
        return 'cannot find sym'
    nodes = sym.nodes
    if len(nodes) != 1:
        return f'{len(nodes)} locations'
    node = nodes[0]
    fname, linenum = node.filename, node.linenr
    cwd = os.getcwd()
    if cwd and fname.startswith(cwd):
        fname = fname[len(cwd) + 1:]
    file_line = f' at {fname}:{linenum}'
    data = read_file(fname)
    if data[linenum - 1] != f'config {imply_config}':
        return None, 0, f'bad sym format {data[linenum]}{file_line})'
    return fname, linenum, f'adding{file_line}'

def add_imply_rule(config, fname, linenum):
    """Add a new 'imply' option to a Kconfig

    Args:
        config (str): config option to add an imply for (without CONFIG_ prefix)
        fname (str): Kconfig filename to update
        linenum (int): Line number to place the 'imply' before

    Returns:
        Message indicating the result
    """
    file_line = f' at {fname}:{linenum}'
    data = read_file(fname)
    linenum -= 1

    for offset, line in enumerate(data[linenum:]):
        if line.strip().startswith('help') or not line:
            data.insert(linenum + offset, f'\timply {config}')
            write_file(fname, data)
            return f'added{file_line}'

    return 'could not insert%s'

(IMPLY_MIN_2, IMPLY_TARGET, IMPLY_CMD, IMPLY_NON_ARCH_BOARD) = (
    1, 2, 4, 8)

IMPLY_FLAGS = {
    'min2': [IMPLY_MIN_2, 'Show options which imply >2 boards (normally >5)'],
    'target': [IMPLY_TARGET, 'Allow CONFIG_TARGET_... options to imply'],
    'cmd': [IMPLY_CMD, 'Allow CONFIG_CMD_... to imply'],
    'non-arch-board': [
        IMPLY_NON_ARCH_BOARD,
        'Allow Kconfig options outside arch/ and /board/ to imply'],
}


def read_database():
    """Read in the config database

    Returns:
        tuple:
            set of all config options seen (each a str)
            set of all defconfigs seen (each a str)
            dict of configs for each defconfig:
                key: defconfig name, e.g. "MPC8548CDS_legacy_defconfig"
                value: dict:
                    key: CONFIG option
                    value: Value of option
            dict of defconfigs for each config:
                key: CONFIG option
                value: set of boards using that option

    """
    configs = {}

    # key is defconfig name, value is dict of (CONFIG_xxx, value)
    config_db = {}

    # Set of all config options we have seen
    all_configs = set()

    # Set of all defconfigs we have seen
    all_defconfigs = set()

    defconfig_db = collections.defaultdict(set)
    for line in read_file(CONFIG_DATABASE):
        line = line.rstrip()
        if not line:  # Separator between defconfigs
            config_db[defconfig] = configs
            all_defconfigs.add(defconfig)
            configs = {}
        elif line[0] == ' ':  # CONFIG line
            config, value = line.strip().split('=', 1)
            configs[config] = value
            defconfig_db[config].add(defconfig)
            all_configs.add(config)
        else:  # New defconfig
            defconfig = line

    return all_configs, all_defconfigs, config_db, defconfig_db


def do_imply_config(config_list, add_imply, imply_flags, skip_added,
                    check_kconfig=True, find_superset=False):
    """Find CONFIG options which imply those in the list

    Some CONFIG options can be implied by others and this can help to reduce
    the size of the defconfig files. For example, CONFIG_X86 implies
    CONFIG_CMD_IRQ, so we can put 'imply CMD_IRQ' under 'config X86' and
    all x86 boards will have that option, avoiding adding CONFIG_CMD_IRQ to
    each of the x86 defconfig files.

    This function uses the qconfig database to find such options. It
    displays a list of things that could possibly imply those in the list.
    The algorithm ignores any that start with CONFIG_TARGET since these
    typically refer to only a few defconfigs (often one). It also does not
    display a config with less than 5 defconfigs.

    The algorithm works using sets. For each target config in config_list:
        - Get the set 'defconfigs' which use that target config
        - For each config (from a list of all configs):
            - Get the set 'imply_defconfig' of defconfigs which use that config
            -
            - If imply_defconfigs contains anything not in defconfigs then
              this config does not imply the target config

    Params:
        config_list: List of CONFIG options to check (each a string)
        add_imply: Automatically add an 'imply' for each config.
        imply_flags: Flags which control which implying configs are allowed
           (IMPLY_...)
        skip_added: Don't show options which already have an imply added.
        check_kconfig: Check if implied symbols already have an 'imply' or
            'select' for the target config, and show this information if so.
        find_superset: True to look for configs which are a superset of those
            already found. So for example if CONFIG_EXYNOS5 implies an option,
            but CONFIG_EXYNOS covers a larger set of defconfigs and also
            implies that option, this will drop the former in favour of the
            latter. In practice this option has not proved very used.

    Note the terminoloy:
        config - a CONFIG_XXX options (a string, e.g. 'CONFIG_CMD_EEPROM')
        defconfig - a defconfig file (a string, e.g. 'configs/snow_defconfig')
    """
    kconf = KconfigScanner().conf if check_kconfig else None
    if add_imply and add_imply != 'all':
        add_imply = add_imply.split(',')

    all_configs, all_defconfigs, _, defconfig_db = read_database()

    # Work through each target config option in turn, independently
    for config in config_list:
        defconfigs = defconfig_db.get(config)
        if not defconfigs:
            print(f'{config} not found in any defconfig')
            continue

        # Get the set of defconfigs without this one (since a config cannot
        # imply itself)
        non_defconfigs = all_defconfigs - defconfigs
        num_defconfigs = len(defconfigs)
        print(f'{config} found in {num_defconfigs}/{len(all_configs)} defconfigs')

        # This will hold the results: key=config, value=defconfigs containing it
        imply_configs = {}
        rest_configs = all_configs - set([config])

        # Look at every possible config, except the target one
        for imply_config in rest_configs:
            if 'ERRATUM' in imply_config:
                continue
            if not imply_flags & IMPLY_CMD:
                if 'CONFIG_CMD' in imply_config:
                    continue
            if not imply_flags & IMPLY_TARGET:
                if 'CONFIG_TARGET' in imply_config:
                    continue

            # Find set of defconfigs that have this config
            imply_defconfig = defconfig_db[imply_config]

            # Get the intersection of this with defconfigs containing the
            # target config
            common_defconfigs = imply_defconfig & defconfigs

            # Get the set of defconfigs containing this config which DO NOT
            # also contain the taret config. If this set is non-empty it means
            # that this config affects other defconfigs as well as (possibly)
            # the ones affected by the target config. This means it implies
            # things we don't want to imply.
            not_common_defconfigs = imply_defconfig & non_defconfigs
            if not_common_defconfigs:
                continue

            # If there are common defconfigs, imply_config may be useful
            if common_defconfigs:
                skip = False
                if find_superset:
                    for prev in list(imply_configs.keys()):
                        prev_count = len(imply_configs[prev])
                        count = len(common_defconfigs)
                        if (prev_count > count and
                            (imply_configs[prev] & common_defconfigs ==
                            common_defconfigs)):
                            # skip imply_config because prev is a superset
                            skip = True
                            break
                        if count > prev_count:
                            # delete prev because imply_config is a superset
                            del imply_configs[prev]
                if not skip:
                    imply_configs[imply_config] = common_defconfigs

        # Now we have a dict imply_configs of configs which imply each config
        # The value of each dict item is the set of defconfigs containing that
        # config. Rank them so that we print the configs that imply the largest
        # number of defconfigs first.
        ranked_iconfigs = sorted(imply_configs,
                            key=lambda k: len(imply_configs[k]), reverse=True)
        kconfig_info = ''
        cwd = os.getcwd()
        add_list = collections.defaultdict(list)
        for iconfig in ranked_iconfigs:
            num_common = len(imply_configs[iconfig])

            # Don't bother if there are less than 5 defconfigs affected.
            if num_common < (2 if imply_flags & IMPLY_MIN_2 else 5):
                continue
            missing = defconfigs - imply_configs[iconfig]
            missing_str = ', '.join(missing) if missing else 'all'
            missing_str = ''
            show = True
            if kconf:
                sym = find_kconfig_rules(kconf, config[CONFIG_LEN:],
                                         iconfig[CONFIG_LEN:])
                kconfig_info = ''
                if sym:
                    nodes = sym.nodes
                    if len(nodes) == 1:
                        fname, linenum = nodes[0].filename, nodes[0].linenr
                        if cwd and fname.startswith(cwd):
                            fname = fname[len(cwd) + 1:]
                        kconfig_info = f'{fname}:{linenum}'
                        if skip_added:
                            show = False
                else:
                    sym = kconf.syms.get(iconfig[CONFIG_LEN:])
                    fname = ''
                    if sym:
                        nodes = sym.nodes
                        if len(nodes) == 1:
                            fname, linenum = nodes[0].filename, nodes[0].linenr
                            if cwd and fname.startswith(cwd):
                                fname = fname[len(cwd) + 1:]
                    in_arch_board = not sym or (fname.startswith('arch') or
                                                fname.startswith('board'))
                    if (not in_arch_board and
                        not imply_flags & IMPLY_NON_ARCH_BOARD):
                        continue

                    if add_imply and (add_imply == 'all' or
                                      iconfig in add_imply):
                        fname, linenum, kconfig_info = (check_imply_rule(kconf,
                                config[CONFIG_LEN:], iconfig[CONFIG_LEN:]))
                        if fname:
                            add_list[fname].append(linenum)

            if show and kconfig_info != 'skip':
                print(f'{num_common:5d} : '
                      f'{iconfig.ljust(30):-30s}{kconfig_info:-25s} {missing_str}')

        # Having collected a list of things to add, now we add them. We process
        # each file from the largest line number to the smallest so that
        # earlier additions do not affect our line numbers. E.g. if we added an
        # imply at line 20 it would change the position of each line after
        # that.
        for fname, linenums in add_list.items():
            for linenum in sorted(linenums, reverse=True):
                add_imply_rule(config[CONFIG_LEN:], fname, linenum)

def defconfig_matches(configs, re_match):
    """Check if any CONFIG option matches a regex

    The match must be complete, i.e. from the start to end of the CONFIG option.

    Args:
        configs (dict): Dict of CONFIG options:
            key: CONFIG option
            value: Value of option
        re_match (re.Pattern): Match to check

    Returns:
        bool: True if any CONFIG matches the regex
    """
    for cfg in configs:
        if re_match.fullmatch(cfg):
            return True
    return False

def do_find_config(config_list):
    """Find boards with a given combination of CONFIGs

    Params:
        config_list: List of CONFIG options to check (each a regex consisting
            of a config option, with or without a CONFIG_ prefix. If an option
            is preceded by a tilde (~) then it must be false, otherwise it must
            be true)
    """
    _, all_defconfigs, config_db, _ = read_database()

    # Start with all defconfigs
    out = all_defconfigs

    # Work through each config in turn
    for item in config_list:
        # Get the real config name and whether we want this config or not
        cfg = item
        want = True
        if cfg[0] == '~':
            want = False
            cfg = cfg[1:]

        # Search everything that is still in the running. If it has a config
        # that we want, or doesn't have one that we don't, add it into the
        # running for the next stage
        in_list = out
        out = set()
        re_match = re.compile(cfg)
        for defc in in_list:
            has_cfg = defconfig_matches(config_db[defc], re_match)
            if has_cfg == want:
                out.add(defc)
    print(f'{len(out)} matches')
    print(' '.join(item.split('_defconfig')[0] for item in out))


def prefix_config(cfg):
    """Prefix a config with CONFIG_ if needed

    This handles ~ operator, which indicates that the CONFIG should be disabled

    >>> prefix_config('FRED')
    'CONFIG_FRED'
    >>> prefix_config('CONFIG_FRED')
    'CONFIG_FRED'
    >>> prefix_config('~FRED')
    '~CONFIG_FRED'
    >>> prefix_config('~CONFIG_FRED')
    '~CONFIG_FRED'
    >>> prefix_config('A123')
    'CONFIG_A123'
    """
    oper = ''
    if cfg[0] == '~':
        oper = cfg[0]
        cfg = cfg[1:]
    if not cfg.startswith('CONFIG_'):
        cfg = 'CONFIG_' + cfg
    return oper + cfg


RE_MK_CONFIGS = re.compile(r'CONFIG_(\$\(SPL_(?:TPL_)?\))?([A-Za-z0-9_]*)')
RE_IFDEF = re.compile(r'(ifdef|ifndef)')
RE_C_CONFIGS = re.compile(r'CONFIG_([A-Za-z0-9_]*)')
RE_CONFIG_IS = re.compile(r'CONFIG_IS_ENABLED\(([A-Za-z0-9_]*)\)')

class ConfigUse:
    def __init__(self, cfg, is_spl, fname, rest):
        self.cfg = cfg
        self.is_spl = is_spl
        self.fname = fname
        self.rest = rest

    def __hash__(self):
        return hash((self.cfg, self.is_spl))

def scan_makefiles(fnames):
    """Scan Makefiles looking for Kconfig options

    Looks for uses of CONFIG options in Makefiles

    Args:
        fnames (list of tuple):
            str: Makefile filename where the option was found
            str: Line of the Makefile

    Returns:
        tuple:
            dict: all_uses
                key (ConfigUse): object
                value (list of str): matching lines
            dict: Uses by filename
                key (str): filename
                value (set of ConfigUse): uses in that filename

    >>> RE_MK_CONFIGS.search('CONFIG_FRED').groups()
    (None, 'FRED')
    >>> RE_MK_CONFIGS.search('CONFIG_$(SPL_)MARY').groups()
    ('$(SPL_)', 'MARY')
    >>> RE_MK_CONFIGS.search('CONFIG_$(SPL_TPL_)MARY').groups()
    ('$(SPL_TPL_)', 'MARY')
    """
    all_uses = collections.defaultdict(list)
    fname_uses = {}
    for fname, rest in fnames:
        m_iter = RE_MK_CONFIGS.finditer(rest)
        for mat in m_iter:
            real_opt = mat.group(2)
            if real_opt == '':
                continue
            is_spl = False
            if mat.group(1):
                is_spl = True
            use = ConfigUse(real_opt, is_spl, fname, rest)
            if fname not in fname_uses:
                fname_uses[fname] = set()
            fname_uses[fname].add(use)
            all_uses[use].append(rest)
    return all_uses, fname_uses


def scan_src_files(fnames):
    """Scan source files (other than Makefiles) looking for Kconfig options

    Looks for uses of CONFIG options

    Args:
        fnames (list of tuple):
            str: Makefile filename where the option was found
            str: Line of the Makefile

    Returns:
        tuple:
            dict: all_uses
                key (ConfigUse): object
                value (list of str): matching lines
            dict: Uses by filename
                key (str): filename
                value (set of ConfigUse): uses in that filename

    >>> RE_C_CONFIGS.search('CONFIG_FRED').groups()
    ('FRED',)
    >>> RE_CONFIG_IS.search('CONFIG_IS_ENABLED(MARY)').groups()
    ('MARY',)
    >>> RE_CONFIG_IS.search('#if CONFIG_IS_ENABLED(OF_PLATDATA)').groups()
    ('OF_PLATDATA',)
    """
    fname = None
    rest = None

    def add_uses(m_iter, is_spl):
        for mat in m_iter:
            real_opt = mat.group(1)
            if real_opt == '':
                continue
            use = ConfigUse(real_opt, is_spl, fname, rest)
            if fname not in fname_uses:
                fname_uses[fname] = set()
            fname_uses[fname].add(use)
            all_uses[use].append(rest)

    all_uses = collections.defaultdict(list)
    fname_uses = {}
    for fname, rest in fnames:
        m_iter = RE_C_CONFIGS.finditer(rest)
        add_uses(m_iter, False)

        m_iter2 = RE_CONFIG_IS.finditer(rest)
        add_uses(m_iter2, True)

    return all_uses, fname_uses


MODE_NORMAL, MODE_SPL, MODE_PROPER = range(3)

def do_scan_source(path, do_update):
    """Scan the source tree for Kconfig inconsistencies

    Args:
        path (str): Path to source tree
        do_update (bool) : True to write to scripts/kconf_... files
    """
    def is_not_proper(name):
        for prefix in SPL_PREFIXES:
            if name.startswith(prefix):
                return name[len(prefix):]
        return False

    def check_not_found(all_uses, spl_mode):
        """Check for Kconfig options mentioned in the source but not in Kconfig

        Args:
            all_uses (dict):
                key (ConfigUse): object
                value (list of str): matching lines
            spl_mode (int): If MODE_SPL, look at source code which implies
                an SPL_ option, but for which there is none;
                for MOD_PROPER, look at source code which implies a Proper
                option (i.e. use of CONFIG_IS_ENABLED() or $(SPL_) or
                $(SPL_TPL_) but for which there none;
                if MODE_NORMAL, ignore SPL

        Returns:
            dict:
                key (str): CONFIG name (without 'CONFIG_' prefix
                value (list of ConfigUse): List of uses of this CONFIG
        """
        # Make sure we know about all the options
        not_found = collections.defaultdict(list)
        for use, _ in all_uses.items():
            name = use.cfg
            if name in IGNORE_SYMS:
                continue
            check = True

            if spl_mode == MODE_SPL:
                check = use.is_spl

                # If it is an SPL symbol, try prepending all SPL_ prefixes to
                # find at least one SPL symbol
                if use.is_spl:
                    for prefix in SPL_PREFIXES:
                        try_name = prefix + name
                        sym = kconf.syms.get(try_name)
                        if sym:
                            break
                    if not sym:
                        not_found[f'SPL_{name}'].append(use)
                    continue
            elif spl_mode == MODE_PROPER:
                # Try to find the Proper version of this symbol, i.e. without
                # the SPL_ prefix
                proper_name = is_not_proper(name)
                if proper_name:
                    name = proper_name
                elif not use.is_spl:
                    check = False
            else: # MODE_NORMAL
                sym = kconf.syms.get(name)
                if not sym:
                    proper_name = is_not_proper(name)
                    if proper_name:
                        name = proper_name
                    sym = kconf.syms.get(name)
                if not sym:
                    for prefix in SPL_PREFIXES:
                        try_name = prefix + name
                        sym = kconf.syms.get(try_name)
                        if sym:
                            break
                if not sym:
                    not_found[name].append(use)
                continue

            sym = kconf.syms.get(name)
            if not sym and check:
                not_found[name].append(use)
        return not_found

    def show_uses(uses):
        """Show a list of uses along with their filename and code snippet

        Args:
            uses (dict):
                key (str): CONFIG name (without 'CONFIG_' prefix
                value (list of ConfigUse): List of uses of this CONFIG
        """
        for name in sorted(uses):
            print(f'{name}: ', end='')
            for i, use in enumerate(uses[name]):
                print(f'{"   " if i else ""}{use.fname}: {use.rest.strip()}')


    print('Scanning Kconfig')
    kconf = KconfigScanner().conf
    print(f'Scanning source in {path}')
    args = ['git', 'grep', '-E', r'IS_ENABLED|\bCONFIG']
    with subprocess.Popen(args, stdout=subprocess.PIPE) as proc:
        out, _ = proc.communicate()
    lines = out.splitlines()
    re_fname = re.compile('^([^:]*):(.*)')
    src_list = []
    mk_list = []
    for line in lines:
        linestr = line.decode('utf-8')
        m_fname = re_fname.search(linestr)
        if not m_fname:
            continue
        fname, rest = m_fname.groups()
        dirname, leaf = os.path.split(fname)
        root, ext = os.path.splitext(leaf)
        if ext == '.autoconf':
            pass
        elif ext in ['.c', '.h', '.S', '.lds', '.dts', '.dtsi', '.asl', '.cfg',
                     '.env', '.tmpl']:
            src_list.append([fname, rest])
        elif 'Makefile' in root or ext == '.mk':
            mk_list.append([fname, rest])
        elif ext in ['.yml', '.sh', '.py', '.awk', '.pl', '.rst', '', '.sed']:
            pass
        elif 'Kconfig' in root or 'Kbuild' in root:
            pass
        elif 'README' in root:
            pass
        elif dirname in ['configs']:
            pass
        elif dirname.startswith('doc') or dirname.startswith('scripts/kconfig'):
            pass
        else:
            print(f'Not sure how to handle file {fname}')

    # Scan the Makefiles
    all_uses, _ = scan_makefiles(mk_list)

    spl_not_found = set()
    proper_not_found = set()

    # Make sure we know about all the options
    print('\nCONFIG options present in Makefiles but not Kconfig:')
    not_found = check_not_found(all_uses, MODE_NORMAL)
    show_uses(not_found)

    print('\nCONFIG options present in Makefiles but not Kconfig (SPL):')
    not_found = check_not_found(all_uses, MODE_SPL)
    show_uses(not_found)
    spl_not_found |= {is_not_proper(key) or key for key in not_found.keys()}

    print('\nCONFIG options used as Proper in Makefiles but without a non-SPL_ variant:')
    not_found = check_not_found(all_uses, MODE_PROPER)
    show_uses(not_found)
    proper_not_found |= {not_found.keys()}

    # Scan the source code
    all_uses, _ = scan_src_files(src_list)

    # Make sure we know about all the options
    print('\nCONFIG options present in source but not Kconfig:')
    not_found = check_not_found(all_uses, MODE_NORMAL)
    show_uses(not_found)

    print('\nCONFIG options present in source but not Kconfig (SPL):')
    not_found = check_not_found(all_uses, MODE_SPL)
    show_uses(not_found)
    spl_not_found |= {is_not_proper(key) or key for key in not_found.keys()}

    print('\nCONFIG options used as Proper in source but without a non-SPL_ variant:')
    not_found = check_not_found(all_uses, MODE_PROPER)
    show_uses(not_found)
    proper_not_found |= {not_found.keys()}

    print('\nCONFIG options used as SPL but without an SPL_ variant:')
    for item in sorted(spl_not_found):
        print(f'   {item}')

    print('\nCONFIG options used as Proper but without a non-SPL_ variant:')
    for item in sorted(proper_not_found):
        print(f'   {item}')

    # Write out the updated information
    if do_update:
        with open(os.path.join(path, 'scripts', 'conf_nospl'), 'w',
                  encoding='utf-8') as out:
            print('# These options should not be enabled in SPL builds\n',
                  file=out)
            for item in sorted(spl_not_found):
                print(item, file=out)
        with open(os.path.join(path, 'scripts', 'conf_noproper'), 'w',
                  encoding='utf-8') as out:
            print('# These options should not be enabled in Proper builds\n',
                  file=out)
            for item in sorted(proper_not_found):
                print(item, file=out)


def main():
    try:
        cpu_count = multiprocessing.cpu_count()
    except NotImplementedError:
        cpu_count = 1

    epilog = '''Move config options from headers to defconfig files. See
doc/develop/moveconfig.rst for documentation.'''

    parser = ArgumentParser(epilog=epilog)
    # Add arguments here
    parser.add_argument('-a', '--add-imply', type=str, default='',
                      help='comma-separated list of CONFIG options to add '
                      "an 'imply' statement to for the CONFIG in -i")
    parser.add_argument('-A', '--skip-added', action='store_true', default=False,
                      help="don't show options which are already marked as "
                      'implying others')
    parser.add_argument('-b', '--build-db', action='store_true', default=False,
                      help='build a CONFIG database')
    parser.add_argument('-C', '--commit', action='store_true', default=False,
                      help='Create a git commit for the operation')
    parser.add_argument('--nocolour', action='store_true', default=False,
                      help="don't display the log in colour")
    parser.add_argument('-d', '--defconfigs', type=str,
                      help='a file containing a list of defconfigs to move, '
                      "one per line (for example 'snow_defconfig') "
                      "or '-' to read from stdin")
    parser.add_argument('-e', '--exit-on-error', action='store_true',
                      default=False,
                      help='exit immediately on any error')
    parser.add_argument('-f', '--find', action='store_true', default=False,
                      help='Find boards with a given config combination')
    parser.add_argument('-i', '--imply', action='store_true', default=False,
                      help='find options which imply others')
    parser.add_argument('-I', '--imply-flags', type=str, default='',
                      help="control the -i option ('help' for help")
    parser.add_argument('-j', '--jobs', type=int, default=cpu_count,
                      help='the number of jobs to run simultaneously')
    parser.add_argument('-n', '--dry-run', action='store_true', default=False,
                      help='perform a trial run (show log with no changes)')
    parser.add_argument('-r', '--git-ref', type=str,
                      help='the git ref to clone for building the autoconf.mk')
    parser.add_argument('-s', '--force-sync', action='store_true', default=False,
                      help='force sync by savedefconfig')
    parser.add_argument('-S', '--spl', action='store_true', default=False,
                      help='parse config options defined for SPL build')
    parser.add_argument('--scan-source', action='store_true', default=False,
                      help='scan source for uses of CONFIG options')
    parser.add_argument('-t', '--test', action='store_true', default=False,
                      help='run unit tests')
    parser.add_argument('-y', '--yes', action='store_true', default=False,
                      help="respond 'yes' to any prompts")
    parser.add_argument('-u', '--update', action='store_true', default=False,
                      help="update scripts/ files (use with --scan-source)")
    parser.add_argument('-v', '--verbose', action='store_true', default=False,
                      help='show any build errors as boards are built')
    parser.add_argument('configs', nargs='*')

    args = parser.parse_args()

    if args.test:
        sys.argv = [sys.argv[0]]
        fail, _ = doctest.testmod()
        if fail:
            return 1
        unittest.main()

    col = terminal.Color(terminal.COLOR_NEVER if args.nocolour
                         else terminal.COLOR_IF_TERMINAL)

    if args.scan_source:
        do_scan_source(os.getcwd(), args.update)
        return 0

    if not any((args.force_sync, args.build_db, args.imply, args.find)):
        parser.print_usage()
        sys.exit(1)

    # prefix the option name with CONFIG_ if missing
    configs = [prefix_config(cfg) for cfg in args.configs]

    check_top_directory()

    if args.imply:
        imply_flags = 0
        if args.imply_flags == 'all':
            imply_flags = -1

        elif args.imply_flags:
            for flag in args.imply_flags.split(','):
                bad = flag not in IMPLY_FLAGS
                if bad:
                    print(f"Invalid flag '{flag}'")
                if flag == 'help' or bad:
                    print("Imply flags: (separate with ',')")
                    for name, info in IMPLY_FLAGS.items():
                        print(f' {name:-15s}: {info[1]}')
                    parser.print_usage()
                    sys.exit(1)
                imply_flags |= IMPLY_FLAGS[flag][0]

        do_imply_config(configs, args.add_imply, imply_flags, args.skip_added)
        return 0

    if args.find:
        do_find_config(configs)
        return 0

    # We are either building the database or forcing a sync of defconfigs
    config_db = {}
    db_queue = queue.Queue()
    dbt = DatabaseThread(config_db, db_queue)
    dbt.daemon = True
    dbt.start()

    check_clean_directory()
    bsettings.setup('')
    toolchains = toolchain.Toolchains()
    toolchains.GetSettings()
    toolchains.Scan(verbose=False)
    progress = move_config(toolchains, args, db_queue, col)
    db_queue.join()

    if args.commit:
        subprocess.call(['git', 'add', '-u'])
        if configs:
            msg = 'Convert %s %sto Kconfig' % (configs[0],
                    'et al ' if len(configs) > 1 else '')
            msg += ('\n\nThis converts the following to Kconfig:\n   %s\n' %
                    '\n   '.join(configs))
        else:
            msg = 'configs: Resync with savedefconfig'
            msg += '\n\nRsync all defconfig files using moveconfig.py'
        subprocess.call(['git', 'commit', '-s', '-m', msg])

    failed = progress.total - progress.good
    failure = f'{failed} failed, ' if failed else ''
    if args.build_db:
        with open(CONFIG_DATABASE, 'w', encoding='utf-8') as outf:
            for defconfig, configs in config_db.items():
                outf.write(f'{defconfig}\n')
                for config in sorted(configs.keys()):
                    outf.write(f'   {config}={configs[config]}\n')
                outf.write('\n')
        print(col.build(
            col.RED if failed else col.GREEN,
            f'{failure}{len(config_db)} boards written to {CONFIG_DATABASE}'))
    else:
        if failed:
            print(col.build(col.RED, f'{failure}see {FAILED_LIST}', True))
        else:
            # Add enough spaces to overwrite the progress indicator
            print(col.build(
                col.GREEN, f'{progress.total} processed        ', bright=True))

    return 0


if __name__ == '__main__':
    sys.exit(main())
