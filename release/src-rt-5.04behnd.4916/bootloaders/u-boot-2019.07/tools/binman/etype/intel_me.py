# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel Management Engine binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_me(Entry_blob):
    """Entry containing an Intel Management Engine (ME) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains code used by the SoC that is required to make it work.
    The Management Engine is like a background task that runs things that are
    not clearly documented, but may include keyboard, deplay and network
    access. For platform that use ME it is not possible to disable it. U-Boot
    does not directly execute code in the ME binary.

    A typical filename is 'me.bin'.

    See README.x86 for information about x86 binary blobs.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
