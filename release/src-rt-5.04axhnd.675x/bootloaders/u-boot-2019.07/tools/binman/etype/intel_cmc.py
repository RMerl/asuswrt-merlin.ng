# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel Chip Microcode binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_cmc(Entry_blob):
    """Entry containing an Intel Chipset Micro Code (CMC) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains microcode for some devices in a special format. An
    example filename is 'Microcode/C0_22211.BIN'.

    See README.x86 for information about x86 binary blobs.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
