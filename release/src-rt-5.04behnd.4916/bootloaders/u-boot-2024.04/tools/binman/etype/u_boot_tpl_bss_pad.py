# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for BSS padding for tpl/u-boot-tpl.bin. This padding
# can be added after the TPL binary to ensure that anything concatenated
# to it will appear to TPL to be at the end of BSS rather than the start.
#

from binman import elf
from binman.entry import Entry
from binman.etype.blob import Entry_blob
from u_boot_pylib import tools

class Entry_u_boot_tpl_bss_pad(Entry_blob):
    """U-Boot TPL binary padded with a BSS region

    Properties / Entry arguments:
        None

    This holds the padding added after the TPL binary to cover the BSS (Block
    Started by Symbol) region. This region holds the various variables used by
    TPL. It is set to 0 by TPL when it starts up. If you want to append data to
    the TPL image (such as a device tree file), you must pad out the BSS region
    to avoid the data overlapping with U-Boot variables. This entry is useful in
    that case. It automatically pads out the entry size to cover both the code,
    data and BSS.

    The contents of this entry will a certain number of zero bytes, determined
    by __bss_size

    The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
    binman uses that to look up the BSS address.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def ObtainContents(self):
        fname = tools.get_input_filename('tpl/u-boot-tpl')
        bss_size = elf.GetSymbolAddress(fname, '__bss_size')
        if bss_size is None:
            self.Raise('Expected __bss_size symbol in tpl/u-boot-tpl')
        self.SetContents(tools.get_bytes(0, bss_size))
        return True
