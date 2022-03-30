# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Handle various things related to ELF images
#

from collections import namedtuple, OrderedDict
import command
import os
import re
import struct

import tools

# This is enabled from control.py
debug = False

Symbol = namedtuple('Symbol', ['section', 'address', 'size', 'weak'])


def GetSymbols(fname, patterns):
    """Get the symbols from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        None, if the file does not exist, or Dict:
          key: Name of symbol
          value: Hex value of symbol
    """
    stdout = command.Output('objdump', '-t', fname, raise_on_error=False)
    lines = stdout.splitlines()
    if patterns:
        re_syms = re.compile('|'.join(patterns))
    else:
        re_syms = None
    syms = {}
    syms_started = False
    for line in lines:
        if not line or not syms_started:
            if 'SYMBOL TABLE' in line:
                syms_started = True
            line = None  # Otherwise code coverage complains about 'continue'
            continue
        if re_syms and not re_syms.search(line):
            continue

        space_pos = line.find(' ')
        value, rest = line[:space_pos], line[space_pos + 1:]
        flags = rest[:7]
        parts = rest[7:].split()
        section, size =  parts[:2]
        if len(parts) > 2:
            name = parts[2]
            syms[name] = Symbol(section, int(value, 16), int(size,16),
                                flags[1] == 'w')

    # Sort dict by address
    return OrderedDict(sorted(syms.iteritems(), key=lambda x: x[1].address))

def GetSymbolAddress(fname, sym_name):
    """Get a value of a symbol from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        Symbol value (as an integer) or None if not found
    """
    syms = GetSymbols(fname, [sym_name])
    sym = syms.get(sym_name)
    if not sym:
        return None
    return sym.address

def LookupAndWriteSymbols(elf_fname, entry, section):
    """Replace all symbols in an entry with their correct values

    The entry contents is updated so that values for referenced symbols will be
    visible at run time. This is done by finding out the symbols offsets in the
    entry (using the ELF file) and replacing them with values from binman's data
    structures.

    Args:
        elf_fname: Filename of ELF image containing the symbol information for
            entry
        entry: Entry to process
        section: Section which can be used to lookup symbol values
    """
    fname = tools.GetInputFilename(elf_fname)
    syms = GetSymbols(fname, ['image', 'binman'])
    if not syms:
        return
    base = syms.get('__image_copy_start')
    if not base:
        return
    for name, sym in syms.iteritems():
        if name.startswith('_binman'):
            msg = ("Section '%s': Symbol '%s'\n   in entry '%s'" %
                   (section.GetPath(), name, entry.GetPath()))
            offset = sym.address - base.address
            if offset < 0 or offset + sym.size > entry.contents_size:
                raise ValueError('%s has offset %x (size %x) but the contents '
                                 'size is %x' % (entry.GetPath(), offset,
                                                 sym.size, entry.contents_size))
            if sym.size == 4:
                pack_string = '<I'
            elif sym.size == 8:
                pack_string = '<Q'
            else:
                raise ValueError('%s has size %d: only 4 and 8 are supported' %
                                 (msg, sym.size))

            # Look up the symbol in our entry tables.
            value = section.LookupSymbol(name, sym.weak, msg)
            if value is not None:
                value += base.address
            else:
                value = -1
                pack_string = pack_string.lower()
            value_bytes = struct.pack(pack_string, value)
            if debug:
                print('%s:\n   insert %s, offset %x, value %x, length %d' %
                      (msg, name, offset, value, len(value_bytes)))
            entry.data = (entry.data[:offset] + value_bytes +
                        entry.data[offset + sym.size:])
