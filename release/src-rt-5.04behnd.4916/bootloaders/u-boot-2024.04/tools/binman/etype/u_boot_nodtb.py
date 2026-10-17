# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for 'u-boot-nodtb.bin'
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot_nodtb(Entry_blob):
    """U-Boot flat binary without device tree appended

    Properties / Entry arguments:
        - filename: Filename to include (default 'u-boot-nodtb.bin')

    This is the U-Boot binary, containing relocation information to allow it
    to relocate itself at runtime. It does not include a device tree blob at
    the end of it so normally cannot work without it. You can add a u-boot-dtb
    entry after this one, or use a u-boot entry instead, normally expands to a
    section containing u-boot and u-boot-dtb
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'u-boot-nodtb.bin'
