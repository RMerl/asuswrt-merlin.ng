# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for spl/u-boot-spl.bin
#

import elf

from entry import Entry
from blob import Entry_blob

class Entry_u_boot_spl(Entry_blob):
    """U-Boot SPL binary

    Properties / Entry arguments:
        - filename: Filename of u-boot-spl.bin (default 'spl/u-boot-spl.bin')

    This is the U-Boot SPL (Secondary Program Loader) binary. This is a small
    binary which loads before U-Boot proper, typically into on-chip SRAM. It is
    responsible for locating, loading and jumping to U-Boot. Note that SPL is
    not relocatable so must be loaded to the correct address in SRAM, or written
    to run from the correct address if direct flash execution is possible (e.g.
    on x86 devices).

    SPL can access binman symbols at runtime. See:

        'Access to binman entry offsets at run time (symbols)'

    in the binman README for more information.

    The ELF file 'spl/u-boot-spl' must also be available for this to work, since
    binman uses that to look up symbols to write into the SPL binary.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
        self.elf_fname = 'spl/u-boot-spl'

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl.bin'

    def WriteSymbols(self, section):
        elf.LookupAndWriteSymbols(self.elf_fname, self, section)
