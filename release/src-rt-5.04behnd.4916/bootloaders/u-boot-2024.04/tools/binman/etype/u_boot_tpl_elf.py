# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot TPL ELF image
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_tpl_elf(Entry_blob):
    """U-Boot TPL ELF image

    Properties / Entry arguments:
        - filename: Filename of TPL u-boot (default 'tpl/u-boot-tpl')

    This is the U-Boot TPL ELF image. It does not include a device tree but can
    be relocated to any address for execution.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'tpl/u-boot-tpl'

    def GetDefaultFilename(self):
        return 'tpl/u-boot-tpl'
