# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for 'u-boot-vpl-nodtb.bin'
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_vpl_nodtb(Entry_blob):
    """VPL binary without device tree appended

    Properties / Entry arguments:
        - filename: Filename to include (default 'vpl/u-boot-vpl-nodtb.bin')

    This is the U-Boot VPL binary, It does not include a device tree blob at
    the end of it so may not be able to work without it, assuming VPL needs
    a device tree to operate on your platform. You can add a u_boot_vpl_dtb
    entry after this one, or use a u_boot_vpl entry instead, which normally
    expands to a section containing u-boot-vpl-dtb, u-boot-vpl-bss-pad and
    u-boot-vpl-dtb

    VPL can access binman symbols at runtime. See :ref:`binman_fdt`.

    The ELF file 'vpl/u-boot-vpl' must also be available for this to work, since
    binman uses that to look up symbols to write into the VPL binary.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'vpl/u-boot-vpl'

    def GetDefaultFilename(self):
        return 'vpl/u-boot-vpl-nodtb.bin'
