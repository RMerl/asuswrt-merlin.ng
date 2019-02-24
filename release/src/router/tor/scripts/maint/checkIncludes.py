#!/usr/bin/python3
# Copyright 2018 The Tor Project, Inc.  See LICENSE file for licensing info.

"""This script looks through all the directories for files matching *.c or
   *.h, and checks their #include directives to make sure that only "permitted"
   headers are included.

   Any #include directives with angle brackets (like #include <stdio.h>) are
   ignored -- only directives with quotes (like #include "foo.h") are
   considered.

   To decide what includes are permitted, this script looks at a .may_include
   file in each directory.  This file contains empty lines, #-prefixed
   comments, filenames (like "lib/foo/bar.h") and file globs (like lib/*/*.h)
   for files that are permitted.
"""


from __future__ import print_function

import fnmatch
import os
import re
import sys

# Global: Have there been any errors?
trouble = False

if sys.version_info[0] <= 2:
    def open_file(fname):
        return open(fname, 'r')
else:
    def open_file(fname):
        return open(fname, 'r', encoding='utf-8')

def err(msg):
    """ Declare that an error has happened, and remember that there has
        been an error. """
    global trouble
    trouble = True
    print(msg, file=sys.stderr)

def fname_is_c(fname):
    """ Return true iff 'fname' is the name of a file that we should
        search for possibly disallowed #include directives. """
    return fname.endswith(".h") or fname.endswith(".c")

INCLUDE_PATTERN = re.compile(r'\s*#\s*include\s+"([^"]*)"')
RULES_FNAME = ".may_include"

class Rules(object):
    """ A 'Rules' object is the parsed version of a .may_include file. """
    def __init__(self, dirpath):
        self.dirpath = dirpath
        self.patterns = []
        self.usedPatterns = set()

    def addPattern(self, pattern):
        self.patterns.append(pattern)

    def includeOk(self, path):
        for pattern in self.patterns:
            if fnmatch.fnmatchcase(path, pattern):
                self.usedPatterns.add(pattern)
                return True
        return False

    def applyToLines(self, lines, context=""):
        lineno = 0
        for line in lines:
            lineno += 1
            m = INCLUDE_PATTERN.match(line)
            if m:
                include = m.group(1)
                if not self.includeOk(include):
                    err("Forbidden include of {} on line {}{}".format(
                        include, lineno, context))

    def applyToFile(self, fname):
        with open_file(fname) as f:
            #print(fname)
            self.applyToLines(iter(f), " of {}".format(fname))

    def noteUnusedRules(self):
        for p in self.patterns:
            if p not in self.usedPatterns:
                print("Pattern {} in {} was never used.".format(p, self.dirpath))

def load_include_rules(fname):
    """ Read a rules file from 'fname', and return it as a Rules object. """
    result = Rules(os.path.split(fname)[0])
    with open_file(fname) as f:
        for line in f:
            line = line.strip()
            if line.startswith("#") or not line:
                continue
            result.addPattern(line)
    return result

list_unused = False

for dirpath, dirnames, fnames in os.walk("src"):
    if ".may_include" in fnames:
        rules = load_include_rules(os.path.join(dirpath, RULES_FNAME))
        for fname in fnames:
            if fname_is_c(fname):
                rules.applyToFile(os.path.join(dirpath,fname))
        if list_unused:
            rules.noteUnusedRules()

if trouble:
    err(
"""To change which includes are allowed in a C file, edit the {}
files in its enclosing directory.""".format(RULES_FNAME))
    sys.exit(1)
