#!/usr/bin/env python2
# SPDX-License-Identifier: GPL-2.0+
#
# Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
#

"""
Fill the "Commit" and "Removed" fields of doc/README.scrapyard

The file doc/README.scrapyard is used to keep track of removed boards.

When we remove support for boards, we are supposed to add entries to
doc/README.scrapyard leaving "Commit" and "Removed" fields blank.

The "Commit" field is the commit hash in which the board was removed
and the "Removed" is the date at which the board was removed.  Those
two are known only after the board removal patch was applied, thus they
need to be filled in later.

This effectively means that the person who removes other boards is
supposed to fill in the blank fields before adding new entries to
doc/README.scrapyard.

That is a really tedious task that should be automated.
This script fills the blank fields of doc/README.scrapyard for you!

Usage:

The "Commit" and "Removed" fields must be "-".  The other fields should
have already been filled in by a former commit.

Run
    scripts/fill_scrapyard.py
"""

import os
import subprocess
import sys
import tempfile

DOC='doc/README.scrapyard'

def get_last_modify_commit(file, line_num):
    """Get the commit that last modified the given line.

    This function runs "git blame" against the given line of the given
    file and returns the commit hash that last modified it.

    Arguments:
      file: the file to be git-blame'd.
      line_num: the line number to be git-blame'd.  This line number
                starts from 1, not 0.

    Returns:
      Commit hash that last modified the line.  The number of digits is
      long enough to form a unique commit.
    """
    result = subprocess.check_output(['git', 'blame', '-L',
                                      '%d,%d' % (line_num, line_num), file])
    commit = result.split()[0]

    if commit[0] == '^':
        sys.exit('%s: line %d: ' % (file, line_num) +
                 'this line was modified before the beginning of git history')

    if commit == '0' * len(commit):
        sys.exit('%s: line %d: locally modified\n' % (file, line_num) +
                 'Please run this script in a clean repository.')

    return commit

def get_committer_date(commit):
    """Get the committer date of the given commit.

    This function returns the date when the given commit was applied.

    Arguments:
      commit: commit-ish object.

    Returns:
      The committer date of the given commit in the form YY-MM-DD.
    """
    committer_date = subprocess.check_output(['git', 'show', '-s',
                                              '--format=%ci', commit])
    return committer_date.split()[0]

def move_to_topdir():
    """Change directory to the top of the git repository.

    Or, exit with an error message if called out of a git repository.
    """
    try:
        toplevel = subprocess.check_output(['git', 'rev-parse',
                                            '--show-toplevel'])
    except subprocess.CalledProcessError:
        sys.exit('Please run in a git repository.')

    # strip '\n'
    toplevel = toplevel.rstrip()

    # Change the current working directory to the toplevel of the respository
    # for our easier life.
    os.chdir(toplevel)

class TmpFile:

    """Useful class to handle a temporary file.

    tempfile.mkstemp() is often used to create a unique temporary file,
    but what is inconvenient is that the caller is responsible for
    deleting the file when done with it.

    Even when the caller errors out on the way, the temporary file must
    be deleted somehow.  The idea here is that we delete the file in
    the destructor of this class because the destructor is always
    invoked when the instance of the class is freed.
    """

    def __init__(self):
        """Constructor - create a temporary file"""
        fd, self.filename = tempfile.mkstemp()
        self.file = os.fdopen(fd, 'w')

    def __del__(self):
        """Destructor - delete the temporary file"""
        try:
            os.remove(self.filename)
        except:
            pass

def main():
    move_to_topdir()

    line_num = 1

    tmpfile = TmpFile()
    for line in open(DOC):
        tmp = line.split(None, 5)
        modified = False

        if len(tmp) >= 5:
            # fill "Commit" field
            if tmp[3] == '-':
                tmp[3] = get_last_modify_commit(DOC, line_num)
                modified = True
            # fill "Removed" field
            if tmp[4] == '-':
                tmp[4] = get_committer_date(tmp[3])
            if modified:
                line  = tmp[0].ljust(17)
                line += tmp[1].ljust(12)
                line += tmp[2].ljust(15)
                line += tmp[3].ljust(12)
                line += tmp[4].ljust(12)
                if len(tmp) >= 6:
                    line += tmp[5]
                line = line.rstrip() + '\n'

        tmpfile.file.write(line)
        line_num += 1

    os.rename(tmpfile.filename, DOC)

if __name__ == '__main__':
    main()
