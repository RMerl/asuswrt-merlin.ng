#!/usr/bin/env python

# Implementation of various source code metrics.
# These are currently ad-hoc string operations and regexps.
# We might want to use a proper static analysis library in the future, if we want to get more advanced metrics.

# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import re

def get_file_len(f):
    """Get file length of file"""
    i = -1
    for i, l in enumerate(f):
        pass
    return i + 1

def get_include_count(f):
    """Get number of #include statements in the file"""
    include_count = 0
    for line in f:
        if re.match(r'\s*#\s*include', line):
            include_count += 1
    return include_count

def get_function_lines(f):
    """
    Return iterator which iterates over functions and returns (function name, function lines)
    """

    # Skip lines that look like they are defining functions with these
    # names: they aren't real function definitions.
    REGEXP_CONFUSE_TERMS = {"MOCK_IMPL", "MOCK_DECL", "HANDLE_DECL",
                            "ENABLE_GCC_WARNINGS", "ENABLE_GCC_WARNING",
                            "DUMMY_TYPECHECK_INSTANCE",
                            "DISABLE_GCC_WARNING", "DISABLE_GCC_WARNINGS"}

    in_function = False
    found_openbrace = False
    for lineno, line in enumerate(f):
        if not in_function:
            # find the start of a function
            m = re.match(r'^([a-zA-Z_][a-zA-Z_0-9]*),?\(', line)
            if m:
                func_name = m.group(1)
                if func_name in REGEXP_CONFUSE_TERMS:
                    continue
                func_start = lineno
                in_function = True
        elif not found_openbrace and line.startswith("{"):
            found_openbrace = True
            func_start = lineno
        else:
            # Find the end of a function
            if line.startswith("}"):
                n_lines = lineno - func_start + 1
                in_function = False
                found_openbrace = False
                yield (func_name, n_lines)
