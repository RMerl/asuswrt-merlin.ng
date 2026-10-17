# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for an TPL binary with an embedded microcode pointer
#

import struct

from binman.entry import Entry
from binman.etype.blob import Entry_blob
from binman.etype.u_boot_with_ucode_ptr import Entry_u_boot_with_ucode_ptr
from u_boot_pylib import command
from u_boot_pylib import tools

class Entry_u_boot_tpl_with_ucode_ptr(Entry_u_boot_with_ucode_ptr):
    """U-Boot TPL with embedded microcode pointer

    See Entry_u_boot_ucode for full details of the entries involved in this
    process.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'tpl/u-boot-tpl'

    def GetDefaultFilename(self):
        return 'tpl/u-boot-tpl-nodtb.bin'
