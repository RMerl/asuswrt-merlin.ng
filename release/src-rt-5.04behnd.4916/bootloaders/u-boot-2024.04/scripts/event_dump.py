#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+

"""Decode the evspy_info linker list in a U-Boot ELF image"""

from argparse import ArgumentParser
import os
import re
import struct
import sys

our_path = os.path.dirname(os.path.realpath(__file__))
src_path = os.path.dirname(our_path)

sys.path.insert(1, os.path.join(our_path, '../tools'))

from binman import elf
from u_boot_pylib import tools

# A typical symbol looks like this:
#   _u_boot_list_2_evspy_info_2_EVT_MISC_INIT_F_3_sandbox_misc_init_f
PREFIX_FULL = '_u_boot_list_2_evspy_info_2_'
PREFIX_SIMPLE = '_u_boot_list_2_evspy_info_simple_2_'
RE_EVTYPE_FULL = re.compile('%s(.*)_3_.*' % PREFIX_FULL)
RE_EVTYPE_SIMPLE = re.compile('%s(.*)_3_.*' % PREFIX_SIMPLE)

def show_sym(fname, data, endian, evtype, sym):
    """Show information about an evspy entry

    Args:
        fname (str): Filename of ELF file
        data (bytes): Data for this symbol
        endian (str): Endianness to use ('little', 'big', 'auto')
        evtype (str): Event type, e.g. 'MISC_INIT_F'
        sym (elf.Symbol): Symbol to show
    """
    def _unpack_val(sym_data, offset):
        start = offset * func_size
        val_data = sym_data[start:start + func_size]
        fmt = '%s%s' % ('>' if endian == 'big' else '<',
                        'L' if func_size == 4 else 'Q')
        val = struct.unpack(fmt, val_data)[0]
        return val

    # Get the data, which is a struct evspy_info
    sym_data = data[sym.offset:sym.offset + sym.size]

    # Figure out the word size of the struct
    func_size = 4 if sym.size < 16 else 8

    # Read the function name for evspy_info->func
    while True:
        # Switch to big-endian if we see a failure
        func_addr = _unpack_val(sym_data, 0)
        func_name = elf.GetSymbolFromAddress(fname, func_addr)
        if not func_name and endian == 'auto':
            endian = 'big'
        else:
            break
    has_id = sym.size in [12, 24]
    if has_id:
        # Find the address of evspy_info->id in the ELF
        id_addr = _unpack_val(sym_data, 2)

        # Get the file offset for that address
        id_ofs = elf.GetFileOffset(fname, id_addr)

        # Read out a nul-terminated string
        id_data = data[id_ofs:id_ofs + 80]
        pos = id_data.find(0)
        if pos:
            id_data = id_data[:pos]
        id_str = id_data.decode('utf-8')
    else:
        id_str = None

    # Find the file/line for the function
    cmd = ['addr2line', '-e', fname, '%x' % func_addr]
    out = tools.run(*cmd).strip()

    # Drop the full path if it is the current directory
    if out.startswith(src_path):
        out = out[len(src_path) + 1:]
    print('%-20s  %-30s  %s' % (evtype, id_str or f'f:{func_name}', out))

def show_event_spy_list(fname, endian):
    """Show a the event-spy- list from a U-Boot image

    Args:
        fname (str): Filename of ELF file
        endian (str): Endianness to use ('little', 'big', 'auto')
    """
    syms = elf.GetSymbolFileOffset(fname, [PREFIX_FULL, PREFIX_SIMPLE])
    data = tools.read_file(fname)
    print('%-20s  %-30s  %s' % ('Event type', 'Id', 'Source location'))
    print('%-20s  %-30s  %s' % ('-' * 20, '-' * 30, '-' * 30))
    for name, sym in syms.items():
        m_evtype = RE_EVTYPE_FULL.search(name)
        if not m_evtype:
            m_evtype = RE_EVTYPE_SIMPLE.search(name)
        evtype = m_evtype .group(1)
        show_sym(fname, data, endian, evtype, sym)

def main(argv):
    """Main program

    Args:
        argv (list of str): List of program arguments, excluding arvg[0]
    """
    epilog = 'Show a list of even spies in a U-Boot EFL file'
    parser = ArgumentParser(epilog=epilog)
    parser.add_argument('elf', type=str, help='ELF file to decode')
    parser.add_argument('-e', '--endian', type=str, default='auto',
                        help='Big-endian image')
    args = parser.parse_args(argv)
    show_event_spy_list(args.elf, args.endian)

if __name__ == "__main__":
    main(sys.argv[1:])
