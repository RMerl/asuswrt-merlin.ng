# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for vpl/u-boot-vpl.bin
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_vpl(Entry_blob):
    """U-Boot VPL binary

    Properties / Entry arguments:
        - filename: Filename of u-boot-vpl.bin (default 'vpl/u-boot-vpl.bin')

    This is the U-Boot VPL (Verifying Program Loader) binary. This is a small
    binary which loads before SPL, typically into on-chip SRAM. It is
    responsible for locating, loading and jumping to SPL, the next-stage
    loader. Note that VPL is not relocatable so must be loaded to the correct
    address in SRAM, or written to run from the correct address if direct
    flash execution is possible (e.g. on x86 devices).

    SPL can access binman symbols at runtime. See :ref:`binman_fdt`.

    in the binman README for more information.

    The ELF file 'vpl/u-boot-vpl' must also be available for this to work, since
    binman uses that to look up symbols to write into the VPL binary.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'vpl/u-boot-vpl'

    def GetDefaultFilename(self):
        return 'vpl/u-boot-vpl.bin'
