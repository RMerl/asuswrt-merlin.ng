#! /usr/bin/python3
# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
#

"""
Script to remove boards

Usage:
   rmboard.py <board_name>...

A single commit is created for each board removed.

Some boards may depend on files provided by another and this will cause
problems, generally the removal of files which should not be removed.

This script works by:
    - Looking through the MAINTAINERS files which mention a board to find out
        what files the board uses
    - Looking through the Kconfig files which mention a board to find one that
        needs to have material removed

Search for ## to update the commit message manually.
"""

import glob
import os
import re
import sys

from u_boot_pylib import command

def rm_kconfig_include(path):
    """Remove a path from Kconfig files

    This function finds the given path in a 'source' statement in a Kconfig
    file and removes that line from the file. This is needed because the path
    is going to be removed, so any reference to it will cause a problem with
    Kconfig parsing.

    The changes are made locally and then added to the git staging area.

    Args:
        path: Path to search for and remove
    """
    cmd = ['git', 'grep', path]
    stdout = command.run_pipe([cmd], capture=True, raise_on_error=False).stdout
    if not stdout:
        return
    fname = stdout.split(':')[0]

    print("Fixing up '%s' to remove reference to '%s'" % (fname, path))
    cmd = ['sed', '-i', '\|%s|d' % path, fname]
    stdout = command.run_pipe([cmd], capture=True).stdout

    cmd = ['git', 'add', fname]
    stdout = command.run_pipe([cmd], capture=True).stdout

def rm_board(board):
    """Create a commit which removes a single board

    This looks up the MAINTAINERS file to file files that need to be removed,
    then removes pieces from the Kconfig files that mention the board.


    Args:
        board: Board name to remove
    """

    # Find all MAINTAINERS and Kconfig files which mention the board
    cmd = ['git', 'grep', '-l', board]
    stdout = command.run_pipe([cmd], capture=True).stdout
    maintain = []
    kconfig = []
    for line in stdout.splitlines():
        line = line.strip()
        if 'MAINTAINERS' in line:
            if line not in maintain:
                maintain.append(line)
        elif 'Kconfig' in line:
            kconfig.append(line)
    paths = []
    cc = []

    # Look through the MAINTAINERS file to find things to remove
    for fname in maintain:
        with open(fname) as fd:
            for line in fd:
                line = line.strip()
                fields = re.split('[ \t]', line, 1)
                if len(fields) == 2:
                    if fields[0] == 'M:':
                        cc.append(fields[1])
                    elif fields[0] == 'F:':
                        paths.append(fields[1].strip())

    # Expand any wildcards in the MAINTAINERS file
    real = []
    for path in paths:
        if path[-1] == '/':
            path = path[:-1]
        if '*' in path:
            globbed = glob.glob(path)
            print("Expanded '%s' to '%s'" % (path, globbed))
            real += globbed
        else:
            real.append(path)

    # Search for Kconfig files in the resulting list. Remove any 'source' lines
    # which reference Kconfig files we want to remove
    for path in real:
        cmd = ['find', path]
        stdout = (command.run_pipe([cmd], capture=True, raise_on_error=False).
                  stdout)
        for fname in stdout.splitlines():
            if fname.endswith('Kconfig'):
                rm_kconfig_include(fname)

    # Remove unwanted files
    cmd = ['git', 'rm', '-r'] + real
    stdout = command.run_pipe([cmd], capture=True).stdout

    ## Change the messages as needed
    msg = '''arm: Remove %s board

This board has not been converted to CONFIG_DM_MMC by the deadline.
Remove it.

''' % board
    for name in cc:
        msg += 'Patch-cc: %s\n' % name

    # Create the commit
    cmd = ['git', 'commit', '-s', '-m', msg]
    stdout = command.run_pipe([cmd], capture=True).stdout

    # Check if the board is mentioned anywhere else. The user will need to deal
    # with this
    cmd = ['git', 'grep', '-il', board]
    print(command.run_pipe([cmd], capture=True, raise_on_error=False).stdout)
    print(' '.join(cmd))

for board in sys.argv[1:]:
    rm_board(board)
