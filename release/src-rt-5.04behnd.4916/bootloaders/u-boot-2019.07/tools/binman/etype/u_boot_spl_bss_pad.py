# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for BSS padding for spl/u-boot-spl.bin. This padding
# can be added after the SPL binary to ensure that anything concatenated
# to it will appear to SPL to be at the end of BSS rather than the start.
#

import command
import elf
from entry import Entry
from blob import Entry_blob
import tools

class Entry_u_boot_spl_bss_pad(Entry_blob):
    """U-Boot SPL binary padded with a BSS region

    Properties / Entry arguments:
        None

    This is similar to u_boot_spl except that padding is added after the SPL
    binary to cover the BSS (Block Started by Symbol) region. This region holds
    the various used by SPL. It is set to 0 by SPL when it starts up. If you
    want to append data to the SPL image (such as a device tree file), you must
    pad out the BSS region to avoid the data overlapping with U-Boot variables.
    This entry is useful in that case. It automatically pads out the entry size
    to cover both the code, data and BSS.

    The ELF file 'spl/u-boot-spl' must also be available for this to work, since
    binman uses that to look up the BSS address.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def ObtainContents(self):
        fname = tools.GetInputFilename('spl/u-boot-spl')
        bss_size = elf.GetSymbolAddress(fname, '__bss_size')
        if not bss_size:
            self.Raise('Expected __bss_size symbol in spl/u-boot-spl')
        self.SetContents(chr(0) * bss_size)
        return True
