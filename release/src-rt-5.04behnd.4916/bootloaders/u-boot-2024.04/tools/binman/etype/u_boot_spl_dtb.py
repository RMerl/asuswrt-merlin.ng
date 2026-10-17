# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree in SPL (Secondary Program Loader)
#

from binman.entry import Entry
from binman.etype.blob_dtb import Entry_blob_dtb

class Entry_u_boot_spl_dtb(Entry_blob_dtb):
    """U-Boot SPL device tree

    Properties / Entry arguments:
        - filename: Filename of u-boot.dtb (default 'spl/u-boot-spl.dtb')

    This is the SPL device tree, containing configuration information for
    SPL. SPL needs this to know what devices are present and which drivers
    to activate.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl.dtb'

    def GetFdtEtype(self):
        return 'u-boot-spl-dtb'
