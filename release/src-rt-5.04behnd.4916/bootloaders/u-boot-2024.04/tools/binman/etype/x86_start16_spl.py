# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for the 16-bit x86 start-up code for U-Boot SPL
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_x86_start16_spl(Entry_blob):
    """x86 16-bit start-up code for SPL

    Properties / Entry arguments:
        - filename: Filename of spl/u-boot-x86-start16-spl.bin (default
            'spl/u-boot-x86-start16-spl.bin')

    x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
    must be placed in the top 64KB of the ROM. The reset code jumps to it. This
    entry holds that code. It is typically placed at offset
    CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
    and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
    U-Boot).

    For 32-bit U-Boot, the 'x86-start16' entry type is used instead.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'spl/u-boot-x86-start16-spl.bin'
