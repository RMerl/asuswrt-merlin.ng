# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for 'u-boot-tpl-nodtb.bin'
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_tpl_nodtb(Entry_blob):
    """TPL binary without device tree appended

    Properties / Entry arguments:
        - filename: Filename to include (default 'tpl/u-boot-tpl-nodtb.bin')

    This is the U-Boot TPL binary, It does not include a device tree blob at
    the end of it so may not be able to work without it, assuming TPL needs
    a device tree to operate on your platform. You can add a u-boot-tpl-dtb
    entry after this one, or use a u-boot-tpl entry instead, which normally
    expands to a section containing u-boot-tpl-dtb, u-boot-tpl-bss-pad and
    u-boot-tpl-dtb

    TPL can access binman symbols at runtime. See :ref:`binman_fdt`.

    in the binman README for more information.

    The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
    binman uses that to look up symbols to write into the TPL binary.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'tpl/u-boot-tpl'

    def GetDefaultFilename(self):
        return 'tpl/u-boot-tpl-nodtb.bin'
