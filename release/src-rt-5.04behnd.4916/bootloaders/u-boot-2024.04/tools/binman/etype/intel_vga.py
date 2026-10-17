# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for x86 VGA ROM binary blob
#

from binman.etype.blob_ext import Entry_blob_ext

class Entry_intel_vga(Entry_blob_ext):
    """Intel Video Graphics Adaptor (VGA) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains code that sets up the integrated graphics subsystem on
    some Intel SoCs. U-Boot executes this when the display is started up.

    This is similar to the VBT file but in a different format.

    See README.x86 for information about Intel binary blobs.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
