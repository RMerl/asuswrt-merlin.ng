# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree with the microcode removed
#

from binman.etype.u_boot_dtb_with_ucode import Entry_u_boot_dtb_with_ucode

class Entry_u_boot_tpl_dtb_with_ucode(Entry_u_boot_dtb_with_ucode):
    """U-Boot TPL with embedded microcode pointer

    This is used when TPL must set up the microcode for U-Boot.

    See Entry_u_boot_ucode for full details of the entries involved in this
    process.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'tpl/u-boot-tpl.dtb'

    def GetFdtEtype(self):
        return 'u-boot-tpl-dtb'
