# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for 'u-boot-nodtb.bin'
#

from entry import Entry
from blob import Entry_blob

class Entry_u_boot_spl_nodtb(Entry_blob):
    """SPL binary without device tree appended

    Properties / Entry arguments:
        - filename: Filename of spl/u-boot-spl-nodtb.bin (default
            'spl/u-boot-spl-nodtb.bin')

    This is the U-Boot SPL binary, It does not include a device tree blob at
    the end of it so may not be able to work without it, assuming SPL needs
    a device tree to operation on your platform. You can add a u_boot_spl_dtb
    entry after this one, or use a u_boot_spl entry instead (which contains
    both SPL and the device tree).
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl-nodtb.bin'
