# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for x86 VGA ROM binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_vga(Entry_blob):
    """Entry containing an Intel Video Graphics Adaptor (VGA) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains code that sets up the integrated graphics subsystem on
    some Intel SoCs. U-Boot executes this when the display is started up.

    This is similar to the VBT file but in a different format.

    See README.x86 for information about Intel binary blobs.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
