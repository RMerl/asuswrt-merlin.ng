# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot VPL ELF image
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_vpl_elf(Entry_blob):
    """U-Boot VPL ELF image

    Properties / Entry arguments:
        - filename: Filename of VPL u-boot (default 'vpl/u-boot-vpl')

    This is the U-Boot VPL ELF image. It does not include a device tree but can
    be relocated to any address for execution.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'vpl/u-boot-vpl'

    def GetDefaultFilename(self):
        return 'vpl/u-boot-vpl'
