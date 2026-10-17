# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree in TPL (Tertiary Program Loader)
#

from binman.entry import Entry
from binman.etype.blob_dtb import Entry_blob_dtb

class Entry_u_boot_tpl_dtb(Entry_blob_dtb):
    """U-Boot TPL device tree

    Properties / Entry arguments:
        - filename: Filename of u-boot.dtb (default 'tpl/u-boot-tpl.dtb')

    This is the TPL device tree, containing configuration information for
    TPL. TPL needs this to know what devices are present and which drivers
    to activate.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'tpl/u-boot-tpl.dtb'

    def GetFdtEtype(self):
        return 'u-boot-tpl-dtb'
