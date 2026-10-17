#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2021 Google LLC
#

"""Changes the functions and class methods in a file to use snake case, updating
other tools which use them"""

from argparse import ArgumentParser
import glob
import os
import re
import subprocess

import camel_case

# Exclude functions with these names
EXCLUDE_NAMES = set(['setUp', 'tearDown', 'setUpClass', 'tearDownClass'])

# Find function definitions in a file
RE_FUNC = re.compile(r' *def (\w+)\(')

# Where to find files that might call the file being converted
FILES_GLOB = 'tools/**/*.py'

def collect_funcs(fname):
    """Collect a list of functions in a file

    Args:
        fname (str): Filename to read

    Returns:
        tuple:
            str: contents of file
            list of str: List of function names
    """
    with open(fname, encoding='utf-8') as inf:
        data = inf.read()
        funcs = RE_FUNC.findall(data)
    return data, funcs

def get_module_name(fname):
    """Convert a filename to a module name

    Args:
        fname (str): Filename to convert, e.g. 'tools/patman/command.py'

    Returns:
        tuple:
            str: Full module name, e.g. 'patman.command'
            str: Leaf module name, e.g. 'command'
            str: Program name, e.g. 'patman'
    """
    parts = os.path.splitext(fname)[0].split('/')[1:]
    module_name = '.'.join(parts)
    return module_name, parts[-1], parts[0]

def process_caller(data, conv, module_name, leaf):
    """Process a file that might call another module

    This converts all the camel-case references in the provided file contents
    with the corresponding snake-case references.

    Args:
        data (str): Contents of file to convert
        conv (dict): Identifies to convert
            key: Current name in camel case, e.g. 'DoIt'
            value: New name in snake case, e.g. 'do_it'
        module_name: Name of module as referenced by the file, e.g.
            'patman.command'
        leaf: Leaf module name, e.g. 'command'

    Returns:
        str: New file contents, or None if it was not modified
    """
    total = 0

    # Update any simple functions calls into the module
    for name, new_name in conv.items():
        newdata, count = re.subn(fr'{leaf}.{name}\(',
                                 f'{leaf}.{new_name}(', data)
        total += count
        data = newdata

    # Deal with files that import symbols individually
    imports = re.findall(fr'from {module_name} import (.*)\n', data)
    for item in imports:
        #print('item', item)
        names = [n.strip() for n in item.split(',')]
        new_names = [conv.get(n) or n for n in names]
        new_line = f"from {module_name} import {', '.join(new_names)}\n"
        data = re.sub(fr'from {module_name} import (.*)\n', new_line, data)
        for name in names:
            new_name = conv.get(name)
            if new_name:
                newdata = re.sub(fr'\b{name}\(', f'{new_name}(', data)
                data = newdata

    # Deal with mocks like:
    # unittest.mock.patch.object(module, 'Function', ...
    for name, new_name in conv.items():
        newdata, count = re.subn(fr"{leaf}, '{name}'",
                                 f"{leaf}, '{new_name}'", data)
        total += count
        data = newdata

    if total or imports:
        return data
    return None

def process_file(srcfile, do_write, commit):
    """Process a file to rename its camel-case functions

    This renames the class methods and functions in a file so that they use
    snake case. Then it updates other modules that call those functions.

    Args:
        srcfile (str): Filename to process
        do_write (bool): True to write back to files, False to do a dry run
        commit (bool): True to create a commit with the changes
    """
    data, funcs = collect_funcs(srcfile)
    module_name, leaf, prog = get_module_name(srcfile)
    #print('module_name', module_name)
    #print(len(funcs))
    #print(funcs[0])
    conv = {}
    for name in funcs:
        if name not in EXCLUDE_NAMES:
            conv[name] = camel_case.to_snake(name)

    # Convert name to new_name in the file
    for name, new_name in conv.items():
        #print(name, new_name)
        # Don't match if it is preceded by a '.', since that indicates that
        # it is calling this same function name but in a different module
        newdata = re.sub(fr'(?<!\.){name}\(', f'{new_name}(', data)
        data = newdata

        # But do allow self.xxx
        newdata = re.sub(fr'self.{name}\(', f'self.{new_name}(', data)
        data = newdata
    if do_write:
        with open(srcfile, 'w', encoding='utf-8') as out:
            out.write(data)

    # Now find all files which use these functions and update them
    for fname in glob.glob(FILES_GLOB, recursive=True):
        with open(fname, encoding='utf-8') as inf:
            data = inf.read()
        newdata = process_caller(fname, conv, module_name, leaf)
        if do_write and newdata:
            with open(fname, 'w', encoding='utf-8') as out:
                out.write(newdata)

    if commit:
        subprocess.call(['git', 'add', '-u'])
        subprocess.call([
            'git', 'commit', '-s', '-m',
            f'''{prog}: Convert camel case in {os.path.basename(srcfile)}

Convert this file to snake case and update all files which use it.
'''])


def main():
    """Main program"""
    epilog = 'Convert camel case function names to snake in a file and callers'
    parser = ArgumentParser(epilog=epilog)
    parser.add_argument('-c', '--commit', action='store_true',
                        help='Add a commit with the changes')
    parser.add_argument('-n', '--dry_run', action='store_true',
                        help='Dry run, do not write back to files')
    parser.add_argument('-s', '--srcfile', type=str, required=True, help='Filename to convert')
    args = parser.parse_args()
    process_file(args.srcfile, not args.dry_run, args.commit)

if __name__ == '__main__':
    main()
