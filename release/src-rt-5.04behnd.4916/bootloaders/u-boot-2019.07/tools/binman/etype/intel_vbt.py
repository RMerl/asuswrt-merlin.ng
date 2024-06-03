# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
#
# Entry-type module for Intel Video BIOS Table binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_vbt(Entry_blob):
    """Entry containing an Intel Video BIOS Table (VBT) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains code that sets up the integrated graphics subsystem on
    some Intel SoCs. U-Boot executes this when the display is started up.

    See README.x86 for information about Intel binary blobs.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
