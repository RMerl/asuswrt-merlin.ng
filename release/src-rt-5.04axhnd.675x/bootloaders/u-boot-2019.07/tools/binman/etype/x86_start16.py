# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for the 16-bit x86 start-up code for U-Boot
#

from entry import Entry
from blob import Entry_blob

class Entry_x86_start16(Entry_blob):
    """x86 16-bit start-up code for U-Boot

    Properties / Entry arguments:
        - filename: Filename of u-boot-x86-16bit.bin (default
            'u-boot-x86-16bit.bin')

    x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
    must be placed at a particular address. This entry holds that code. It is
    typically placed at offset CONFIG_SYS_X86_START16. The code is responsible
    for changing to 32-bit mode and jumping to U-Boot's entry point, which
    requires 32-bit mode (for 32-bit U-Boot).

    For 64-bit U-Boot, the 'x86_start16_spl' entry type is used instead.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def GetDefaultFilename(self):
        return 'u-boot-x86-16bit.bin'
