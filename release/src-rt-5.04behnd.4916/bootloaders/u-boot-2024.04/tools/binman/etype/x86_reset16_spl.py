# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for the 16-bit x86 reset code for U-Boot
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_x86_reset16_spl(Entry_blob):
    """x86 16-bit reset code for U-Boot

    Properties / Entry arguments:
        - filename: Filename of u-boot-x86-reset16.bin (default
            'u-boot-x86-reset16.bin')

    x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
    must be placed at a particular address. This entry holds that code. It is
    typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
    for jumping to the x86-start16 code, which continues execution.

    For 32-bit U-Boot, the 'x86_reset_spl' entry type is used instead.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'spl/u-boot-x86-reset16-spl.bin'
