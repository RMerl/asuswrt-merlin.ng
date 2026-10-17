# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot binary
#

from binman.etype.blob_phase import Entry_blob_phase

class Entry_u_boot_expanded(Entry_blob_phase):
    """U-Boot flat binary broken out into its component parts

    This is a section containing the U-Boot binary and a devicetree. Using this
    entry type automatically creates this section, with the following entries
    in it:

       u-boot-nodtb
       u-boot-dtb

    Having the devicetree separate allows binman to update it in the final
    image, so that the entries positions are provided to the running U-Boot.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'u-boot', 'u-boot-dtb', False)
