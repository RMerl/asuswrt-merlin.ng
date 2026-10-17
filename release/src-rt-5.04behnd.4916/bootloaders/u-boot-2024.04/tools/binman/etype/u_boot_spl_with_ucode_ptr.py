# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for an SPL binary with an embedded microcode pointer
#

import struct

from binman.etype.u_boot_with_ucode_ptr import Entry_u_boot_with_ucode_ptr

class Entry_u_boot_spl_with_ucode_ptr(Entry_u_boot_with_ucode_ptr):
    """U-Boot SPL with embedded microcode pointer

    This is used when SPL must set up the microcode for U-Boot.

    See Entry_u_boot_ucode for full details of the entries involved in this
    process.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, auto_write_symbols=True)
        self.elf_fname = 'spl/u-boot-spl'

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl-nodtb.bin'
