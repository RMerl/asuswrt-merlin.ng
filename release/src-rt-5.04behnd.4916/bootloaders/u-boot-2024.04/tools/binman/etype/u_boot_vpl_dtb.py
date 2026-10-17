# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree in VPL (Verifying Program Loader)
#

from binman.entry import Entry
from binman.etype.blob_dtb import Entry_blob_dtb

class Entry_u_boot_vpl_dtb(Entry_blob_dtb):
    """U-Boot VPL device tree

    Properties / Entry arguments:
        - filename: Filename of u-boot.dtb (default 'vpl/u-boot-vpl.dtb')

    This is the VPL device tree, containing configuration information for
    VPL. VPL needs this to know what devices are present and which drivers
    to activate.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'vpl/u-boot-vpl.dtb'

    def GetFdtEtype(self):
        return 'u-boot-vpl-dtb'
