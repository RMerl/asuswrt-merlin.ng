# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot SPL ELF image
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_spl_elf(Entry_blob):
    """U-Boot SPL ELF image

    Properties / Entry arguments:
        - filename: Filename of SPL u-boot (default 'spl/u-boot-spl')

    This is the U-Boot SPL ELF image. It does not include a device tree but can
    be relocated to any address for execution.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'spl/u-boot-spl'

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl'
