#!/usr/bin/env python2
# SPDX-License-Identifier: GPL-2.0+
#
# Author: Masahiro Yamada <yamada.masahiro@socionext.com>
#

"""
Move config options from headers to defconfig files.

Since Kconfig was introduced to U-Boot, we have worked on moving
config options from headers to Kconfig (defconfig).

This tool intends to help this tremendous work.


Usage
-----

First, you must edit the Kconfig to add the menu entries for the configs
you are moving.

And then run this tool giving CONFIG names you want to move.
For example, if you want to move CONFIG_CMD_USB and CONFIG_SYS_TEXT_BASE,
simply type as follows:

  $ tools/moveconfig.py CONFIG_CMD_USB CONFIG_SYS_TEXT_BASE

The tool walks through all the defconfig files and move the given CONFIGs.

The log is also displayed on the terminal.

The log is printed for each defconfig as follows:

<defconfig_name>
    <action1>
    <action2>
    <action3>
    ...

<defconfig_name> is the name of the defconfig.

<action*> shows what the tool did for that defconfig.
It looks like one of the following:

 - Move 'CONFIG_... '
   This config option was moved to the defconfig

 - CONFIG_... is not defined in Kconfig.  Do nothing.
   The entry for this CONFIG was not found in Kconfig.  The option is not
   defined in the config header, either.  So, this case can be just skipped.

 - CONFIG_... is not defined in Kconfig (suspicious).  Do nothing.
   This option is defined in the config header, but its entry was not found
   in Kconfig.
   There are two common cases:
     - You forgot to create an entry for the CONFIG before running
       this tool, or made a typo in a CONFIG passed to this tool.
     - The entry was hidden due to unmet 'depends on'.
   The tool does not know if the result is reasonable, so please check it
   manually.

 - 'CONFIG_...' is the same as the define in Kconfig.  Do nothing.
   The define in the config header matched the one in Kconfig.
   We do not need to touch it.

 - Compiler is missing.  Do nothing.
   The compiler specified for this architecture was not found
   in your PATH environment.
   (If -e option is passed, the tool exits immediately.)

 - Failed to process.
   An error occurred during processing this defconfig.  Skipped.
   (If -e option is passed, the tool exits immediately on error.)

Finally, you will be asked, Clean up headers? [y/n]:

If you say 'y' here, the unnecessary config defines are removed
from the config headers (include/configs/*.h).
It just uses the regex method, so you should not rely on it.
Just in case, please do 'git diff' to see what happened.


How does it work?
-----------------

This tool runs configuration and builds include/autoconf.mk for every
defconfig.  The config options defined in Kconfig appear in the .config
file (unless they are hidden because of unmet dependency.)
On the other hand, the config options defined by board headers are seen
in include/autoconf.mk.  The tool looks for the specified options in both
of them to decide the appropriate action for the options.  If the given
config option is found in the .config, but its value does not match the
one from the board header, the config option in the .config is replaced
with the define in the board header.  Then, the .config is synced by
"make savedefconfig" and the defconfig is updated with it.

For faster processing, this tool handles multi-threading.  It creates
separate build directories where the out-of-tree build is run.  The
temporary build directories are automatically created and deleted as
needed.  The number of threads are chosen based on the number of the CPU
cores of your system although you can change it via -j (--jobs) option.


Toolchains
----------

Appropriate toolchain are necessary to generate include/autoconf.mk
for all the architectures supported by U-Boot.  Most of them are available
at the kernel.org site, some are not provided by kernel.org. This tool uses
the same tools as buildman, so see that tool for setup (e.g. --fetch-arch).


Tips and trips
--------------

To sync only X86 defconfigs:

   ./tools/moveconfig.py -s -d <(grep -l X86 configs/*)

or:

   grep -l X86 configs/* | ./tools/moveconfig.py -s -d -

To process CONFIG_CMD_FPGAD only for a subset of configs based on path match:

   ls configs/{hrcon*,iocon*,strider*} | \
       ./tools/moveconfig.py -Cy CONFIG_CMD_FPGAD -d -


Finding implied CONFIGs
-----------------------

Some CONFIG options can be implied by others and this can help to reduce
the size of the defconfig files. For example, CONFIG_X86 implies
CONFIG_CMD_IRQ, so we can put 'imply CMD_IRQ' under 'config X86' and
all x86 boards will have that option, avoiding adding CONFIG_CMD_IRQ to
each of the x86 defconfig files.

This tool can help find such configs. To use it, first build a database:

    ./tools/moveconfig.py -b

Then try to query it:

    ./tools/moveconfig.py -i CONFIG_CMD_IRQ
    CONFIG_CMD_IRQ found in 311/2384 defconfigs
    44 : CONFIG_SYS_FSL_ERRATUM_IFC_A002769
    41 : CONFIG_SYS_FSL_ERRATUM_A007075
    31 : CONFIG_SYS_FSL_DDR_VER_44
    28 : CONFIG_ARCH_P1010
    28 : CONFIG_SYS_FSL_ERRATUM_P1010_A003549
    28 : CONFIG_SYS_FSL_ERRATUM_SEC_A003571
    28 : CONFIG_SYS_FSL_ERRATUM_IFC_A003399
    25 : CONFIG_SYS_FSL_ERRATUM_A008044
    22 : CONFIG_ARCH_P1020
    21 : CONFIG_SYS_FSL_DDR_VER_46
    20 : CONFIG_MAX_PIRQ_LINKS
    20 : CONFIG_HPET_ADDRESS
    20 : CONFIG_X86
    20 : CONFIG_PCIE_ECAM_SIZE
    20 : CONFIG_IRQ_SLOT_COUNT
    20 : CONFIG_I8259_PIC
    20 : CONFIG_CPU_ADDR_BITS
    20 : CONFIG_RAMBASE
    20 : CONFIG_SYS_FSL_ERRATUM_A005871
    20 : CONFIG_PCIE_ECAM_BASE
    20 : CONFIG_X86_TSC_TIMER
    20 : CONFIG_I8254_TIMER
    20 : CONFIG_CMD_GETTIME
    19 : CONFIG_SYS_FSL_ERRATUM_A005812
    18 : CONFIG_X86_RUN_32BIT
    17 : CONFIG_CMD_CHIP_CONFIG
    ...

This shows a list of config options which might imply CONFIG_CMD_EEPROM along
with how many defconfigs they cover. From this you can see that CONFIG_X86
implies CONFIG_CMD_EEPROM. Therefore, instead of adding CONFIG_CMD_EEPROM to
the defconfig of every x86 board, you could add a single imply line to the
Kconfig file:

    config X86
        bool "x86 architecture"
        ...
        imply CMD_EEPROM

That will cover 20 defconfigs. Many of the options listed are not suitable as
they are not related. E.g. it would be odd for CONFIG_CMD_GETTIME to imply
CMD_EEPROM.

Using this search you can reduce the size of moveconfig patches.

You can automatically add 'imply' statements in the Kconfig with the -a
option:

    ./tools/moveconfig.py -s -i CONFIG_SCSI \
            -a CONFIG_ARCH_LS1021A,CONFIG_ARCH_LS1043A

This will add 'imply SCSI' to the two CONFIG options mentioned, assuming that
the database indicates that they do actually imply CONFIG_SCSI and do not
already have an 'imply SCSI'.

The output shows where the imply is added:

   18 : CONFIG_ARCH_LS1021A       arch/arm/cpu/armv7/ls102xa/Kconfig:1
   13 : CONFIG_ARCH_LS1043A       arch/arm/cpu/armv8/fsl-layerscape/Kconfig:11
   12 : CONFIG_ARCH_LS1046A       arch/arm/cpu/armv8/fsl-layerscape/Kconfig:31

The first number is the number of boards which can avoid having a special
CONFIG_SCSI option in their defconfig file if this 'imply' is added.
The location at the right is the Kconfig file and line number where the config
appears. For example, adding 'imply CONFIG_SCSI' to the 'config ARCH_LS1021A'
in arch/arm/cpu/armv7/ls102xa/Kconfig at line 1 will help 18 boards to reduce
the size of their defconfig files.

If you want to add an 'imply' to every imply config in the list, you can use

    ./tools/moveconfig.py -s -i CONFIG_SCSI -a all

To control which ones are displayed, use -I <list> where list is a list of
options (use '-I help' to see possible options and their meaning).

To skip showing you options that already have an 'imply' attached, use -A.

When you have finished adding 'imply' options you can regenerate the
defconfig files for affected boards with something like:

    git show --stat | ./tools/moveconfig.py -s -d -

This will regenerate only those defconfigs changed in the current commit.
If you start with (say) 100 defconfigs being changed in the commit, and add
a few 'imply' options as above, then regenerate, hopefully you can reduce the
number of defconfigs changed in the commit.


Available options
-----------------

 -c, --color
   Surround each portion of the log with escape sequences to display it
   in color on the terminal.

 -C, --commit
   Create a git commit with the changes when the operation is complete. A
   standard commit message is used which may need to be edited.

 -d, --defconfigs
  Specify a file containing a list of defconfigs to move.  The defconfig
  files can be given with shell-style wildcards. Use '-' to read from stdin.

 -n, --dry-run
   Perform a trial run that does not make any changes.  It is useful to
   see what is going to happen before one actually runs it.

 -e, --exit-on-error
   Exit immediately if Make exits with a non-zero status while processing
   a defconfig file.

 -s, --force-sync
   Do "make savedefconfig" forcibly for all the defconfig files.
   If not specified, "make savedefconfig" only occurs for cases
   where at least one CONFIG was moved.

 -S, --spl
   Look for moved config options in spl/include/autoconf.mk instead of
   include/autoconf.mk.  This is useful for moving options for SPL build
   because SPL related options (mostly prefixed with CONFIG_SPL_) are
   sometimes blocked by CONFIG_SPL_BUILD ifdef conditionals.

 -H, --headers-only
   Only cleanup the headers; skip the defconfig processing

 -j, --jobs
   Specify the number of threads to run simultaneously.  If not specified,
   the number of threads is the same as the number of CPU cores.

 -r, --git-ref
   Specify the git ref to clone for building the autoconf.mk. If unspecified
   use the CWD. This is useful for when changes to the Kconfig affect the
   default values and you want to capture the state of the defconfig from
   before that change was in effect. If in doubt, specify a ref pre-Kconfig
   changes (use HEAD if Kconfig changes are not committed). Worst case it will
   take a bit longer to run, but will always do the right thing.

 -v, --verbose
   Show any build errors as boards are built

 -y, --yes
   Instead of prompting, automatically go ahead with all operations. This
   includes cleaning up headers, CONFIG_SYS_EXTRA_OPTIONS, the config whitelist
   and the README.

To see the complete list of supported options, run

  $ tools/moveconfig.py -h

"""

import collections
import copy
import difflib
import filecmp
import fnmatch
import glob
import multiprocessing
import optparse
import os
import Queue
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time

sys.path.append(os.path.join(os.path.dirname(__file__), 'buildman'))
sys.path.append(os.path.join(os.path.dirname(__file__), 'patman'))
import bsettings
import kconfiglib
import toolchain

SHOW_GNU_MAKE = 'scripts/show-gnu-make'
SLEEP_TIME=0.03

STATE_IDLE = 0
STATE_DEFCONFIG = 1
STATE_AUTOCONF = 2
STATE_SAVEDEFCONFIG = 3

ACTION_MOVE = 0
ACTION_NO_ENTRY = 1
ACTION_NO_ENTRY_WARN = 2
ACTION_NO_CHANGE = 3

COLOR_BLACK        = '0;30'
COLOR_RED          = '0;31'
COLOR_GREEN        = '0;32'
COLOR_BROWN        = '0;33'
COLOR_BLUE         = '0;34'
COLOR_PURPLE       = '0;35'
COLOR_CYAN         = '0;36'
COLOR_LIGHT_GRAY   = '0;37'
COLOR_DARK_GRAY    = '1;30'
COLOR_LIGHT_RED    = '1;31'
COLOR_LIGHT_GREEN  = '1;32'
COLOR_YELLOW       = '1;33'
COLOR_LIGHT_BLUE   = '1;34'
COLOR_LIGHT_PURPLE = '1;35'
COLOR_LIGHT_CYAN   = '1;36'
COLOR_WHITE        = '1;37'

AUTO_CONF_PATH = 'include/config/auto.conf'
CONFIG_DATABASE = 'moveconfig.db'

CONFIG_LEN = len('CONFIG_')

### helper functions ###
def get_devnull():
    """Get the file object of '/dev/null' device."""
    try:
        devnull = subprocess.DEVNULL # py3k
    except AttributeError:
        devnull = open(os.devnull, 'wb')
    return devnull

def check_top_directory():
    """Exit if we are not at the top of source directory."""
    for f in ('README', 'Licenses'):
        if not os.path.exists(f):
            sys.exit('Please run at the top of source directory.')

def check_clean_directory():
    """Exit if the source tree is not clean."""
    for f in ('.config', 'include/config'):
        if os.path.exists(f):
            sys.exit("source tree is not clean, please run 'make mrproper'")

def get_make_cmd():
    """Get the command name of GNU Make.

    U-Boot needs GNU Make for building, but the command name is not
    necessarily "make". (for example, "gmake" on FreeBSD).
    Returns the most appropriate command name on your system.
    """
    process = subprocess.Popen([SHOW_GNU_MAKE], stdout=subprocess.PIPE)
    ret = process.communicate()
    if process.returncode:
        sys.exit('GNU Make not found')
    return ret[0].rstrip()

def get_matched_defconfig(line):
    """Get the defconfig files that match a pattern

    Args:
        line: Path or filename to match, e.g. 'configs/snow_defconfig' or
            'k2*_defconfig'. If no directory is provided, 'configs/' is
            prepended

    Returns:
        a list of matching defconfig files
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
        defconfigs_file: File containing a list of defconfigs to process, or
            '-' to read the list from stdin

    Returns:
        A list of paths to defconfig files, with no duplicates
    """
    defconfigs = []
    if defconfigs_file == '-':
        fd = sys.stdin
        defconfigs_file = 'stdin'
    else:
        fd = open(defconfigs_file)
    for i, line in enumerate(fd):
        line = line.strip()
        if not line:
            continue # skip blank lines silently
        if ' ' in line:
            line = line.split(' ')[0]  # handle 'git log' input
        matched = get_matched_defconfig(line)
        if not matched:
            print >> sys.stderr, "warning: %s:%d: no defconfig matched '%s'" % \
                                                 (defconfigs_file, i + 1, line)

        defconfigs += matched

    # use set() to drop multiple matching
    return [ defconfig[len('configs') + 1:]  for defconfig in set(defconfigs) ]

def get_all_defconfigs():
    """Get all the defconfig files under the configs/ directory."""
    defconfigs = []
    for (dirpath, dirnames, filenames) in os.walk('configs'):
        dirpath = dirpath[len('configs') + 1:]
        for filename in fnmatch.filter(filenames, '*_defconfig'):
            defconfigs.append(os.path.join(dirpath, filename))

    return defconfigs

def color_text(color_enabled, color, string):
    """Return colored string."""
    if color_enabled:
        # LF should not be surrounded by the escape sequence.
        # Otherwise, additional whitespace or line-feed might be printed.
        return '\n'.join([ '\033[' + color + 'm' + s + '\033[0m' if s else ''
                           for s in string.split('\n') ])
    else:
        return string

def show_diff(a, b, file_path, color_enabled):
    """Show unidified diff.

    Arguments:
      a: A list of lines (before)
      b: A list of lines (after)
      file_path: Path to the file
      color_enabled: Display the diff in color
    """

    diff = difflib.unified_diff(a, b,
                                fromfile=os.path.join('a', file_path),
                                tofile=os.path.join('b', file_path))

    for line in diff:
        if line[0] == '-' and line[1] != '-':
            print color_text(color_enabled, COLOR_RED, line),
        elif line[0] == '+' and line[1] != '+':
            print color_text(color_enabled, COLOR_GREEN, line),
        else:
            print line,

def extend_matched_lines(lines, matched, pre_patterns, post_patterns, extend_pre,
                         extend_post):
    """Extend matched lines if desired patterns are found before/after already
    matched lines.

    Arguments:
      lines: A list of lines handled.
      matched: A list of line numbers that have been already matched.
               (will be updated by this function)
      pre_patterns: A list of regular expression that should be matched as
                    preamble.
      post_patterns: A list of regular expression that should be matched as
                     postamble.
      extend_pre: Add the line number of matched preamble to the matched list.
      extend_post: Add the line number of matched postamble to the matched list.
    """
    extended_matched = []

    j = matched[0]

    for i in matched:
        if i == 0 or i < j:
            continue
        j = i
        while j in matched:
            j += 1
        if j >= len(lines):
            break

        for p in pre_patterns:
            if p.search(lines[i - 1]):
                break
        else:
            # not matched
            continue

        for p in post_patterns:
            if p.search(lines[j]):
                break
        else:
            # not matched
            continue

        if extend_pre:
            extended_matched.append(i - 1)
        if extend_post:
            extended_matched.append(j)

    matched += extended_matched
    matched.sort()

def confirm(options, prompt):
    if not options.yes:
        while True:
            choice = raw_input('{} [y/n]: '.format(prompt))
            choice = choice.lower()
            print choice
            if choice == 'y' or choice == 'n':
                break

        if choice == 'n':
            return False

    return True

def cleanup_empty_blocks(header_path, options):
    """Clean up empty conditional blocks

    Arguments:
      header_path: path to the cleaned file.
      options: option flags.
    """
    pattern = re.compile(r'^\s*#\s*if.*$\n^\s*#\s*endif.*$\n*', flags=re.M)
    with open(header_path) as f:
        data = f.read()

    new_data = pattern.sub('\n', data)

    show_diff(data.splitlines(True), new_data.splitlines(True), header_path,
              options.color)

    if options.dry_run:
        return

    with open(header_path, 'w') as f:
        f.write(new_data)

def cleanup_one_header(header_path, patterns, options):
    """Clean regex-matched lines away from a file.

    Arguments:
      header_path: path to the cleaned file.
      patterns: list of regex patterns.  Any lines matching to these
                patterns are deleted.
      options: option flags.
    """
    with open(header_path) as f:
        lines = f.readlines()

    matched = []
    for i, line in enumerate(lines):
        if i - 1 in matched and lines[i - 1][-2:] == '\\\n':
            matched.append(i)
            continue
        for pattern in patterns:
            if pattern.search(line):
                matched.append(i)
                break

    if not matched:
        return

    # remove empty #ifdef ... #endif, successive blank lines
    pattern_if = re.compile(r'#\s*if(def|ndef)?\W') #  #if, #ifdef, #ifndef
    pattern_elif = re.compile(r'#\s*el(if|se)\W')   #  #elif, #else
    pattern_endif = re.compile(r'#\s*endif\W')      #  #endif
    pattern_blank = re.compile(r'^\s*$')            #  empty line

    while True:
        old_matched = copy.copy(matched)
        extend_matched_lines(lines, matched, [pattern_if],
                             [pattern_endif], True, True)
        extend_matched_lines(lines, matched, [pattern_elif],
                             [pattern_elif, pattern_endif], True, False)
        extend_matched_lines(lines, matched, [pattern_if, pattern_elif],
                             [pattern_blank], False, True)
        extend_matched_lines(lines, matched, [pattern_blank],
                             [pattern_elif, pattern_endif], True, False)
        extend_matched_lines(lines, matched, [pattern_blank],
                             [pattern_blank], True, False)
        if matched == old_matched:
            break

    tolines = copy.copy(lines)

    for i in reversed(matched):
        tolines.pop(i)

    show_diff(lines, tolines, header_path, options.color)

    if options.dry_run:
        return

    with open(header_path, 'w') as f:
        for line in tolines:
            f.write(line)

def cleanup_headers(configs, options):
    """Delete config defines from board headers.

    Arguments:
      configs: A list of CONFIGs to remove.
      options: option flags.
    """
    if not confirm(options, 'Clean up headers?'):
        return

    patterns = []
    for config in configs:
        patterns.append(re.compile(r'#\s*define\s+%s\W' % config))
        patterns.append(re.compile(r'#\s*undef\s+%s\W' % config))

    for dir in 'include', 'arch', 'board':
        for (dirpath, dirnames, filenames) in os.walk(dir):
            if dirpath == os.path.join('include', 'generated'):
                continue
            for filename in filenames:
                if not fnmatch.fnmatch(filename, '*~'):
                    header_path = os.path.join(dirpath, filename)
                    cleanup_one_header(header_path, patterns, options)
                    cleanup_empty_blocks(header_path, options)

def cleanup_one_extra_option(defconfig_path, configs, options):
    """Delete config defines in CONFIG_SYS_EXTRA_OPTIONS in one defconfig file.

    Arguments:
      defconfig_path: path to the cleaned defconfig file.
      configs: A list of CONFIGs to remove.
      options: option flags.
    """

    start = 'CONFIG_SYS_EXTRA_OPTIONS="'
    end = '"\n'

    with open(defconfig_path) as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if line.startswith(start) and line.endswith(end):
            break
    else:
        # CONFIG_SYS_EXTRA_OPTIONS was not found in this defconfig
        return

    old_tokens = line[len(start):-len(end)].split(',')
    new_tokens = []

    for token in old_tokens:
        pos = token.find('=')
        if not (token[:pos] if pos >= 0 else token) in configs:
            new_tokens.append(token)

    if new_tokens == old_tokens:
        return

    tolines = copy.copy(lines)

    if new_tokens:
        tolines[i] = start + ','.join(new_tokens) + end
    else:
        tolines.pop(i)

    show_diff(lines, tolines, defconfig_path, options.color)

    if options.dry_run:
        return

    with open(defconfig_path, 'w') as f:
        for line in tolines:
            f.write(line)

def cleanup_extra_options(configs, options):
    """Delete config defines in CONFIG_SYS_EXTRA_OPTIONS in defconfig files.

    Arguments:
      configs: A list of CONFIGs to remove.
      options: option flags.
    """
    if not confirm(options, 'Clean up CONFIG_SYS_EXTRA_OPTIONS?'):
        return

    configs = [ config[len('CONFIG_'):] for config in configs ]

    defconfigs = get_all_defconfigs()

    for defconfig in defconfigs:
        cleanup_one_extra_option(os.path.join('configs', defconfig), configs,
                                 options)

def cleanup_whitelist(configs, options):
    """Delete config whitelist entries

    Arguments:
      configs: A list of CONFIGs to remove.
      options: option flags.
    """
    if not confirm(options, 'Clean up whitelist entries?'):
        return

    with open(os.path.join('scripts', 'config_whitelist.txt')) as f:
        lines = f.readlines()

    lines = [x for x in lines if x.strip() not in configs]

    with open(os.path.join('scripts', 'config_whitelist.txt'), 'w') as f:
        f.write(''.join(lines))

def find_matching(patterns, line):
    for pat in patterns:
        if pat.search(line):
            return True
    return False

def cleanup_readme(configs, options):
    """Delete config description in README

    Arguments:
      configs: A list of CONFIGs to remove.
      options: option flags.
    """
    if not confirm(options, 'Clean up README?'):
        return

    patterns = []
    for config in configs:
        patterns.append(re.compile(r'^\s+%s' % config))

    with open('README') as f:
        lines = f.readlines()

    found = False
    newlines = []
    for line in lines:
        if not found:
            found = find_matching(patterns, line)
            if found:
                continue

        if found and re.search(r'^\s+CONFIG', line):
            found = False

        if not found:
            newlines.append(line)

    with open('README', 'w') as f:
        f.write(''.join(newlines))


### classes ###
class Progress:

    """Progress Indicator"""

    def __init__(self, total):
        """Create a new progress indicator.

        Arguments:
          total: A number of defconfig files to process.
        """
        self.current = 0
        self.total = total

    def inc(self):
        """Increment the number of processed defconfig files."""

        self.current += 1

    def show(self):
        """Display the progress."""
        print ' %d defconfigs out of %d\r' % (self.current, self.total),
        sys.stdout.flush()


class KconfigScanner:
    """Kconfig scanner."""

    def __init__(self):
        """Scan all the Kconfig files and create a Config object."""
        # Define environment variables referenced from Kconfig
        os.environ['srctree'] = os.getcwd()
        os.environ['UBOOTVERSION'] = 'dummy'
        os.environ['KCONFIG_OBJDIR'] = ''
        self.conf = kconfiglib.Config()


class KconfigParser:

    """A parser of .config and include/autoconf.mk."""

    re_arch = re.compile(r'CONFIG_SYS_ARCH="(.*)"')
    re_cpu = re.compile(r'CONFIG_SYS_CPU="(.*)"')

    def __init__(self, configs, options, build_dir):
        """Create a new parser.

        Arguments:
          configs: A list of CONFIGs to move.
          options: option flags.
          build_dir: Build directory.
        """
        self.configs = configs
        self.options = options
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
        for line in open(self.dotconfig):
            m = self.re_arch.match(line)
            if m:
                arch = m.group(1)
                continue
            m = self.re_cpu.match(line)
            if m:
                cpu = m.group(1)

        if not arch:
            return None

        # fix-up for aarch64
        if arch == 'arm' and cpu == 'armv8':
            arch = 'aarch64'

        return arch

    def parse_one_config(self, config, dotconfig_lines, autoconf_lines):
        """Parse .config, defconfig, include/autoconf.mk for one config.

        This function looks for the config options in the lines from
        defconfig, .config, and include/autoconf.mk in order to decide
        which action should be taken for this defconfig.

        Arguments:
          config: CONFIG name to parse.
          dotconfig_lines: lines from the .config file.
          autoconf_lines: lines from the include/autoconf.mk file.

        Returns:
          A tupple of the action for this defconfig and the line
          matched for the config.
        """
        not_set = '# %s is not set' % config

        for line in autoconf_lines:
            line = line.rstrip()
            if line.startswith(config + '='):
                new_val = line
                break
        else:
            new_val = not_set

        for line in dotconfig_lines:
            line = line.rstrip()
            if line.startswith(config + '=') or line == not_set:
                old_val = line
                break
        else:
            if new_val == not_set:
                return (ACTION_NO_ENTRY, config)
            else:
                return (ACTION_NO_ENTRY_WARN, config)

        # If this CONFIG is neither bool nor trisate
        if old_val[-2:] != '=y' and old_val[-2:] != '=m' and old_val != not_set:
            # tools/scripts/define2mk.sed changes '1' to 'y'.
            # This is a problem if the CONFIG is int type.
            # Check the type in Kconfig and handle it correctly.
            if new_val[-2:] == '=y':
                new_val = new_val[:-1] + '1'

        return (ACTION_NO_CHANGE if old_val == new_val else ACTION_MOVE,
                new_val)

    def update_dotconfig(self):
        """Parse files for the config options and update the .config.

        This function parses the generated .config and include/autoconf.mk
        searching the target options.
        Move the config option(s) to the .config as needed.

        Arguments:
          defconfig: defconfig name.

        Returns:
          Return a tuple of (updated flag, log string).
          The "updated flag" is True if the .config was updated, False
          otherwise.  The "log string" shows what happend to the .config.
        """

        results = []
        updated = False
        suspicious = False
        rm_files = [self.config_autoconf, self.autoconf]

        if self.options.spl:
            if os.path.exists(self.spl_autoconf):
                autoconf_path = self.spl_autoconf
                rm_files.append(self.spl_autoconf)
            else:
                for f in rm_files:
                    os.remove(f)
                return (updated, suspicious,
                        color_text(self.options.color, COLOR_BROWN,
                                   "SPL is not enabled.  Skipped.") + '\n')
        else:
            autoconf_path = self.autoconf

        with open(self.dotconfig) as f:
            dotconfig_lines = f.readlines()

        with open(autoconf_path) as f:
            autoconf_lines = f.readlines()

        for config in self.configs:
            result = self.parse_one_config(config, dotconfig_lines,
                                           autoconf_lines)
            results.append(result)

        log = ''

        for (action, value) in results:
            if action == ACTION_MOVE:
                actlog = "Move '%s'" % value
                log_color = COLOR_LIGHT_GREEN
            elif action == ACTION_NO_ENTRY:
                actlog = "%s is not defined in Kconfig.  Do nothing." % value
                log_color = COLOR_LIGHT_BLUE
            elif action == ACTION_NO_ENTRY_WARN:
                actlog = "%s is not defined in Kconfig (suspicious).  Do nothing." % value
                log_color = COLOR_YELLOW
                suspicious = True
            elif action == ACTION_NO_CHANGE:
                actlog = "'%s' is the same as the define in Kconfig.  Do nothing." \
                         % value
                log_color = COLOR_LIGHT_PURPLE
            elif action == ACTION_SPL_NOT_EXIST:
                actlog = "SPL is not enabled for this defconfig.  Skip."
                log_color = COLOR_PURPLE
            else:
                sys.exit("Internal Error. This should not happen.")

            log += color_text(self.options.color, log_color, actlog) + '\n'

        with open(self.dotconfig, 'a') as f:
            for (action, value) in results:
                if action == ACTION_MOVE:
                    f.write(value + '\n')
                    updated = True

        self.results = results
        for f in rm_files:
            os.remove(f)

        return (updated, suspicious, log)

    def check_defconfig(self):
        """Check the defconfig after savedefconfig

        Returns:
          Return additional log if moved CONFIGs were removed again by
          'make savedefconfig'.
        """

        log = ''

        with open(self.defconfig) as f:
            defconfig_lines = f.readlines()

        for (action, value) in self.results:
            if action != ACTION_MOVE:
                continue
            if not value + '\n' in defconfig_lines:
                log += color_text(self.options.color, COLOR_YELLOW,
                                  "'%s' was removed by savedefconfig.\n" %
                                  value)

        return log


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

    def __init__(self, toolchains, configs, options, progress, devnull,
		 make_cmd, reference_src_dir, db_queue):
        """Create a new process slot.

        Arguments:
          toolchains: Toolchains object containing toolchains.
          configs: A list of CONFIGs to move.
          options: option flags.
          progress: A progress indicator.
          devnull: A file object of '/dev/null'.
          make_cmd: command name of GNU Make.
          reference_src_dir: Determine the true starting config state from this
                             source tree.
          db_queue: output queue to write config info for the database
        """
        self.toolchains = toolchains
        self.options = options
        self.progress = progress
        self.build_dir = tempfile.mkdtemp()
        self.devnull = devnull
        self.make_cmd = (make_cmd, 'O=' + self.build_dir)
        self.reference_src_dir = reference_src_dir
        self.db_queue = db_queue
        self.parser = KconfigParser(configs, options, self.build_dir)
        self.state = STATE_IDLE
        self.failed_boards = set()
        self.suspicious_boards = set()

    def __del__(self):
        """Delete the working directory

        This function makes sure the temporary directory is cleaned away
        even if Python suddenly dies due to error.  It should be done in here
        because it is guaranteed the destructor is always invoked when the
        instance of the class gets unreferenced.

        If the subprocess is still running, wait until it finishes.
        """
        if self.state != STATE_IDLE:
            while self.ps.poll() == None:
                pass
        shutil.rmtree(self.build_dir)

    def add(self, defconfig):
        """Assign a new subprocess for defconfig and add it to the slot.

        If the slot is vacant, create a new subprocess for processing the
        given defconfig and add it to the slot.  Just returns False if
        the slot is occupied (i.e. the current subprocess is still running).

        Arguments:
          defconfig: defconfig name.

        Returns:
          Return True on success or False on failure
        """
        if self.state != STATE_IDLE:
            return False

        self.defconfig = defconfig
        self.log = ''
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

        if self.ps.poll() == None:
            return False

        if self.ps.poll() != 0:
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
            elif self.options.build_db:
                self.do_build_db()
            else:
                self.do_savedefconfig()
        elif self.state == STATE_SAVEDEFCONFIG:
            self.update_defconfig()
        else:
            sys.exit("Internal Error. This should not happen.")

        return True if self.state == STATE_IDLE else False

    def handle_error(self):
        """Handle error cases."""

        self.log += color_text(self.options.color, COLOR_LIGHT_RED,
                               "Failed to process.\n")
        if self.options.verbose:
            self.log += color_text(self.options.color, COLOR_LIGHT_CYAN,
                                   self.ps.stderr.read())
        self.finish(False)

    def do_defconfig(self):
        """Run 'make <board>_defconfig' to create the .config file."""

        cmd = list(self.make_cmd)
        cmd.append(self.defconfig)
        self.ps = subprocess.Popen(cmd, stdout=self.devnull,
                                   stderr=subprocess.PIPE,
                                   cwd=self.current_src_dir)
        self.state = STATE_DEFCONFIG

    def do_autoconf(self):
        """Run 'make AUTO_CONF_PATH'."""

        arch = self.parser.get_arch()
        try:
            toolchain = self.toolchains.Select(arch)
        except ValueError:
            self.log += color_text(self.options.color, COLOR_YELLOW,
                    "Tool chain for '%s' is missing.  Do nothing.\n" % arch)
            self.finish(False)
            return
	env = toolchain.MakeEnvironment(False)

        cmd = list(self.make_cmd)
        cmd.append('KCONFIG_IGNORE_DUPLICATES=1')
        cmd.append(AUTO_CONF_PATH)
        self.ps = subprocess.Popen(cmd, stdout=self.devnull, env=env,
                                   stderr=subprocess.PIPE,
                                   cwd=self.current_src_dir)
        self.state = STATE_AUTOCONF

    def do_build_db(self):
        """Add the board to the database"""
        configs = {}
        with open(os.path.join(self.build_dir, AUTO_CONF_PATH)) as fd:
            for line in fd.readlines():
                if line.startswith('CONFIG'):
                    config, value = line.split('=', 1)
                    configs[config] = value.rstrip()
        self.db_queue.put([self.defconfig, configs])
        self.finish(True)

    def do_savedefconfig(self):
        """Update the .config and run 'make savedefconfig'."""

        (updated, suspicious, log) = self.parser.update_dotconfig()
        if suspicious:
            self.suspicious_boards.add(self.defconfig)
        self.log += log

        if not self.options.force_sync and not updated:
            self.finish(True)
            return
        if updated:
            self.log += color_text(self.options.color, COLOR_LIGHT_GREEN,
                                   "Syncing by savedefconfig...\n")
        else:
            self.log += "Syncing by savedefconfig (forced by option)...\n"

        cmd = list(self.make_cmd)
        cmd.append('savedefconfig')
        self.ps = subprocess.Popen(cmd, stdout=self.devnull,
                                   stderr=subprocess.PIPE)
        self.state = STATE_SAVEDEFCONFIG

    def update_defconfig(self):
        """Update the input defconfig and go back to the idle state."""

        log = self.parser.check_defconfig()
        if log:
            self.suspicious_boards.add(self.defconfig)
            self.log += log
        orig_defconfig = os.path.join('configs', self.defconfig)
        new_defconfig = os.path.join(self.build_dir, 'defconfig')
        updated = not filecmp.cmp(orig_defconfig, new_defconfig)

        if updated:
            self.log += color_text(self.options.color, COLOR_LIGHT_BLUE,
                                   "defconfig was updated.\n")

        if not self.options.dry_run and updated:
            shutil.move(new_defconfig, orig_defconfig)
        self.finish(True)

    def finish(self, success):
        """Display log along with progress and go to the idle state.

        Arguments:
          success: Should be True when the defconfig was processed
                   successfully, or False when it fails.
        """
        # output at least 30 characters to hide the "* defconfigs out of *".
        log = self.defconfig.ljust(30) + '\n'

        log += '\n'.join([ '    ' + s for s in self.log.split('\n') ])
        # Some threads are running in parallel.
        # Print log atomically to not mix up logs from different threads.
        print >> (sys.stdout if success else sys.stderr), log

        if not success:
            if self.options.exit_on_error:
                sys.exit("Exit on error.")
            # If --exit-on-error flag is not set, skip this board and continue.
            # Record the failed board.
            self.failed_boards.add(self.defconfig)

        self.progress.inc()
        self.progress.show()
        self.state = STATE_IDLE

    def get_failed_boards(self):
        """Returns a set of failed boards (defconfigs) in this slot.
        """
        return self.failed_boards

    def get_suspicious_boards(self):
        """Returns a set of boards (defconfigs) with possible misconversion.
        """
        return self.suspicious_boards - self.failed_boards

class Slots:

    """Controller of the array of subprocess slots."""

    def __init__(self, toolchains, configs, options, progress,
		 reference_src_dir, db_queue):
        """Create a new slots controller.

        Arguments:
          toolchains: Toolchains object containing toolchains.
          configs: A list of CONFIGs to move.
          options: option flags.
          progress: A progress indicator.
          reference_src_dir: Determine the true starting config state from this
                             source tree.
          db_queue: output queue to write config info for the database
        """
        self.options = options
        self.slots = []
        devnull = get_devnull()
        make_cmd = get_make_cmd()
        for i in range(options.jobs):
            self.slots.append(Slot(toolchains, configs, options, progress,
				   devnull, make_cmd, reference_src_dir,
				   db_queue))

    def add(self, defconfig):
        """Add a new subprocess if a vacant slot is found.

        Arguments:
          defconfig: defconfig name to be put into.

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

    def show_failed_boards(self):
        """Display all of the failed boards (defconfigs)."""
        boards = set()
        output_file = 'moveconfig.failed'

        for slot in self.slots:
            boards |= slot.get_failed_boards()

        if boards:
            boards = '\n'.join(boards) + '\n'
            msg = "The following boards were not processed due to error:\n"
            msg += boards
            msg += "(the list has been saved in %s)\n" % output_file
            print >> sys.stderr, color_text(self.options.color, COLOR_LIGHT_RED,
                                            msg)

            with open(output_file, 'w') as f:
                f.write(boards)

    def show_suspicious_boards(self):
        """Display all boards (defconfigs) with possible misconversion."""
        boards = set()
        output_file = 'moveconfig.suspicious'

        for slot in self.slots:
            boards |= slot.get_suspicious_boards()

        if boards:
            boards = '\n'.join(boards) + '\n'
            msg = "The following boards might have been converted incorrectly.\n"
            msg += "It is highly recommended to check them manually:\n"
            msg += boards
            msg += "(the list has been saved in %s)\n" % output_file
            print >> sys.stderr, color_text(self.options.color, COLOR_YELLOW,
                                            msg)

            with open(output_file, 'w') as f:
                f.write(boards)

class ReferenceSource:

    """Reference source against which original configs should be parsed."""

    def __init__(self, commit):
        """Create a reference source directory based on a specified commit.

        Arguments:
          commit: commit to git-clone
        """
        self.src_dir = tempfile.mkdtemp()
        print "Cloning git repo to a separate work directory..."
        subprocess.check_output(['git', 'clone', os.getcwd(), '.'],
                                cwd=self.src_dir)
        print "Checkout '%s' to build the original autoconf.mk." % \
            subprocess.check_output(['git', 'rev-parse', '--short', commit]).strip()
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

def move_config(toolchains, configs, options, db_queue):
    """Move config options to defconfig files.

    Arguments:
      configs: A list of CONFIGs to move.
      options: option flags
    """
    if len(configs) == 0:
        if options.force_sync:
            print 'No CONFIG is specified. You are probably syncing defconfigs.',
        elif options.build_db:
            print 'Building %s database' % CONFIG_DATABASE
        else:
            print 'Neither CONFIG nor --force-sync is specified. Nothing will happen.',
    else:
        print 'Move ' + ', '.join(configs),
    print '(jobs: %d)\n' % options.jobs

    if options.git_ref:
        reference_src = ReferenceSource(options.git_ref)
        reference_src_dir = reference_src.get_dir()
    else:
        reference_src_dir = None

    if options.defconfigs:
        defconfigs = get_matched_defconfigs(options.defconfigs)
    else:
        defconfigs = get_all_defconfigs()

    progress = Progress(len(defconfigs))
    slots = Slots(toolchains, configs, options, progress, reference_src_dir,
		  db_queue)

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

    print ''
    slots.show_failed_boards()
    slots.show_suspicious_boards()

def find_kconfig_rules(kconf, config, imply_config):
    """Check whether a config has a 'select' or 'imply' keyword

    Args:
        kconf: Kconfig.Config object
        config: Name of config to check (without CONFIG_ prefix)
        imply_config: Implying config (without CONFIG_ prefix) which may or
            may not have an 'imply' for 'config')

    Returns:
        Symbol object for 'config' if found, else None
    """
    sym = kconf.get_symbol(imply_config)
    if sym:
        for sel in sym.get_selected_symbols() | sym.get_implied_symbols():
            if sel.get_name() == config:
                return sym
    return None

def check_imply_rule(kconf, config, imply_config):
    """Check if we can add an 'imply' option

    This finds imply_config in the Kconfig and looks to see if it is possible
    to add an 'imply' for 'config' to that part of the Kconfig.

    Args:
        kconf: Kconfig.Config object
        config: Name of config to check (without CONFIG_ prefix)
        imply_config: Implying config (without CONFIG_ prefix) which may or
            may not have an 'imply' for 'config')

    Returns:
        tuple:
            filename of Kconfig file containing imply_config, or None if none
            line number within the Kconfig file, or 0 if none
            message indicating the result
    """
    sym = kconf.get_symbol(imply_config)
    if not sym:
        return 'cannot find sym'
    locs = sym.get_def_locations()
    if len(locs) != 1:
        return '%d locations' % len(locs)
    fname, linenum = locs[0]
    cwd = os.getcwd()
    if cwd and fname.startswith(cwd):
        fname = fname[len(cwd) + 1:]
    file_line = ' at %s:%d' % (fname, linenum)
    with open(fname) as fd:
        data = fd.read().splitlines()
    if data[linenum - 1] != 'config %s' % imply_config:
        return None, 0, 'bad sym format %s%s' % (data[linenum], file_line)
    return fname, linenum, 'adding%s' % file_line

def add_imply_rule(config, fname, linenum):
    """Add a new 'imply' option to a Kconfig

    Args:
        config: config option to add an imply for (without CONFIG_ prefix)
        fname: Kconfig filename to update
        linenum: Line number to place the 'imply' before

    Returns:
        Message indicating the result
    """
    file_line = ' at %s:%d' % (fname, linenum)
    data = open(fname).read().splitlines()
    linenum -= 1

    for offset, line in enumerate(data[linenum:]):
        if line.strip().startswith('help') or not line:
            data.insert(linenum + offset, '\timply %s' % config)
            with open(fname, 'w') as fd:
                fd.write('\n'.join(data) + '\n')
            return 'added%s' % file_line

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
};

def do_imply_config(config_list, add_imply, imply_flags, skip_added,
                    check_kconfig=True, find_superset=False):
    """Find CONFIG options which imply those in the list

    Some CONFIG options can be implied by others and this can help to reduce
    the size of the defconfig files. For example, CONFIG_X86 implies
    CONFIG_CMD_IRQ, so we can put 'imply CMD_IRQ' under 'config X86' and
    all x86 boards will have that option, avoiding adding CONFIG_CMD_IRQ to
    each of the x86 defconfig files.

    This function uses the moveconfig database to find such options. It
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
        add_imply = add_imply.split()

    # key is defconfig name, value is dict of (CONFIG_xxx, value)
    config_db = {}

    # Holds a dict containing the set of defconfigs that contain each config
    # key is config, value is set of defconfigs using that config
    defconfig_db = collections.defaultdict(set)

    # Set of all config options we have seen
    all_configs = set()

    # Set of all defconfigs we have seen
    all_defconfigs = set()

    # Read in the database
    configs = {}
    with open(CONFIG_DATABASE) as fd:
        for line in fd.readlines():
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

    # Work through each target config option in tern, independently
    for config in config_list:
        defconfigs = defconfig_db.get(config)
        if not defconfigs:
            print '%s not found in any defconfig' % config
            continue

        # Get the set of defconfigs without this one (since a config cannot
        # imply itself)
        non_defconfigs = all_defconfigs - defconfigs
        num_defconfigs = len(defconfigs)
        print '%s found in %d/%d defconfigs' % (config, num_defconfigs,
                                                len(all_configs))

        # This will hold the results: key=config, value=defconfigs containing it
        imply_configs = {}
        rest_configs = all_configs - set([config])

        # Look at every possible config, except the target one
        for imply_config in rest_configs:
            if 'ERRATUM' in imply_config:
                continue
            if not (imply_flags & IMPLY_CMD):
                if 'CONFIG_CMD' in imply_config:
                    continue
            if not (imply_flags & IMPLY_TARGET):
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
                    for prev in imply_configs.keys():
                        prev_count = len(imply_configs[prev])
                        count = len(common_defconfigs)
                        if (prev_count > count and
                            (imply_configs[prev] & common_defconfigs ==
                            common_defconfigs)):
                            # skip imply_config because prev is a superset
                            skip = True
                            break
                        elif count > prev_count:
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
                    locs = sym.get_def_locations()
                    if len(locs) == 1:
                        fname, linenum = locs[0]
                        if cwd and fname.startswith(cwd):
                            fname = fname[len(cwd) + 1:]
                        kconfig_info = '%s:%d' % (fname, linenum)
                        if skip_added:
                            show = False
                else:
                    sym = kconf.get_symbol(iconfig[CONFIG_LEN:])
                    fname = ''
                    if sym:
                        locs = sym.get_def_locations()
                        if len(locs) == 1:
                            fname, linenum = locs[0]
                            if cwd and fname.startswith(cwd):
                                fname = fname[len(cwd) + 1:]
                    in_arch_board = not sym or (fname.startswith('arch') or
                                                fname.startswith('board'))
                    if (not in_arch_board and
                        not (imply_flags & IMPLY_NON_ARCH_BOARD)):
                        continue

                    if add_imply and (add_imply == 'all' or
                                      iconfig in add_imply):
                        fname, linenum, kconfig_info = (check_imply_rule(kconf,
                                config[CONFIG_LEN:], iconfig[CONFIG_LEN:]))
                        if fname:
                            add_list[fname].append(linenum)

            if show and kconfig_info != 'skip':
                print '%5d : %-30s%-25s %s' % (num_common, iconfig.ljust(30),
                                              kconfig_info, missing_str)

        # Having collected a list of things to add, now we add them. We process
        # each file from the largest line number to the smallest so that
        # earlier additions do not affect our line numbers. E.g. if we added an
        # imply at line 20 it would change the position of each line after
        # that.
        for fname, linenums in add_list.iteritems():
            for linenum in sorted(linenums, reverse=True):
                add_imply_rule(config[CONFIG_LEN:], fname, linenum)


def main():
    try:
        cpu_count = multiprocessing.cpu_count()
    except NotImplementedError:
        cpu_count = 1

    parser = optparse.OptionParser()
    # Add options here
    parser.add_option('-a', '--add-imply', type='string', default='',
                      help='comma-separated list of CONFIG options to add '
                      "an 'imply' statement to for the CONFIG in -i")
    parser.add_option('-A', '--skip-added', action='store_true', default=False,
                      help="don't show options which are already marked as "
                      'implying others')
    parser.add_option('-b', '--build-db', action='store_true', default=False,
                      help='build a CONFIG database')
    parser.add_option('-c', '--color', action='store_true', default=False,
                      help='display the log in color')
    parser.add_option('-C', '--commit', action='store_true', default=False,
                      help='Create a git commit for the operation')
    parser.add_option('-d', '--defconfigs', type='string',
                      help='a file containing a list of defconfigs to move, '
                      "one per line (for example 'snow_defconfig') "
                      "or '-' to read from stdin")
    parser.add_option('-i', '--imply', action='store_true', default=False,
                      help='find options which imply others')
    parser.add_option('-I', '--imply-flags', type='string', default='',
                      help="control the -i option ('help' for help")
    parser.add_option('-n', '--dry-run', action='store_true', default=False,
                      help='perform a trial run (show log with no changes)')
    parser.add_option('-e', '--exit-on-error', action='store_true',
                      default=False,
                      help='exit immediately on any error')
    parser.add_option('-s', '--force-sync', action='store_true', default=False,
                      help='force sync by savedefconfig')
    parser.add_option('-S', '--spl', action='store_true', default=False,
                      help='parse config options defined for SPL build')
    parser.add_option('-H', '--headers-only', dest='cleanup_headers_only',
                      action='store_true', default=False,
                      help='only cleanup the headers')
    parser.add_option('-j', '--jobs', type='int', default=cpu_count,
                      help='the number of jobs to run simultaneously')
    parser.add_option('-r', '--git-ref', type='string',
                      help='the git ref to clone for building the autoconf.mk')
    parser.add_option('-y', '--yes', action='store_true', default=False,
                      help="respond 'yes' to any prompts")
    parser.add_option('-v', '--verbose', action='store_true', default=False,
                      help='show any build errors as boards are built')
    parser.usage += ' CONFIG ...'

    (options, configs) = parser.parse_args()

    if len(configs) == 0 and not any((options.force_sync, options.build_db,
                                      options.imply)):
        parser.print_usage()
        sys.exit(1)

    # prefix the option name with CONFIG_ if missing
    configs = [ config if config.startswith('CONFIG_') else 'CONFIG_' + config
                for config in configs ]

    check_top_directory()

    if options.imply:
        imply_flags = 0
        if options.imply_flags == 'all':
            imply_flags = -1

        elif options.imply_flags:
            for flag in options.imply_flags.split(','):
                bad = flag not in IMPLY_FLAGS
                if bad:
                    print "Invalid flag '%s'" % flag
                if flag == 'help' or bad:
                    print "Imply flags: (separate with ',')"
                    for name, info in IMPLY_FLAGS.iteritems():
                        print ' %-15s: %s' % (name, info[1])
                    parser.print_usage()
                    sys.exit(1)
                imply_flags |= IMPLY_FLAGS[flag][0]

        do_imply_config(configs, options.add_imply, imply_flags,
                        options.skip_added)
        return

    config_db = {}
    db_queue = Queue.Queue()
    t = DatabaseThread(config_db, db_queue)
    t.setDaemon(True)
    t.start()

    if not options.cleanup_headers_only:
        check_clean_directory()
	bsettings.Setup('')
        toolchains = toolchain.Toolchains()
        toolchains.GetSettings()
        toolchains.Scan(verbose=False)
        move_config(toolchains, configs, options, db_queue)
        db_queue.join()

    if configs:
        cleanup_headers(configs, options)
        cleanup_extra_options(configs, options)
        cleanup_whitelist(configs, options)
        cleanup_readme(configs, options)

    if options.commit:
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

    if options.build_db:
        with open(CONFIG_DATABASE, 'w') as fd:
            for defconfig, configs in config_db.iteritems():
                fd.write('%s\n' % defconfig)
                for config in sorted(configs.keys()):
                    fd.write('   %s=%s\n' % (config, configs[config]))
                fd.write('\n')

if __name__ == '__main__':
    main()
