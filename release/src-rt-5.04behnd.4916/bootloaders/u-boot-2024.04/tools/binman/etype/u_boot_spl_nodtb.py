# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for 'u-boot-spl-nodtb.bin'
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_spl_nodtb(Entry_blob):
    """SPL binary without device tree appended

    Properties / Entry arguments:
        - filename: Filename to include (default 'spl/u-boot-spl-nodtb.bin')

    This is the U-Boot SPL binary, It does not include a device tree blob at
    the end of it so may not be able to work without it, assuming SPL needs
    a device tree to operate on your platform. You can add a u-boot-spl-dtb
    entry after this one, or use a u-boot-spl entry instead' which normally
    expands to a section containing u-boot-spl-dtb, u-boot-spl-bss-pad and
    u-boot-spl-dtb

    SPL can access binman symbols at runtime. See :ref:`binman_fdt`.

    in the binman README for more information.

    The ELF file 'spl/u-boot-spl' must also be available for this to work, since
    binman uses that to look up symbols to write into the SPL binary.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'spl/u-boot-spl'

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl-nodtb.bin'
