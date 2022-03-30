# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree files
#

import state

from entry import Entry
from blob import Entry_blob

class Entry_blob_dtb(Entry_blob):
    """A blob that holds a device tree

    This is a blob containing a device tree. The contents of the blob are
    obtained from the list of available device-tree files, managed by the
    'state' module.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def ObtainContents(self):
        """Get the device-tree from the list held by the 'state' module"""
        self._filename = self.GetDefaultFilename()
        self._pathname, data = state.GetFdtContents(self._filename)
        self.SetContents(data)
        return True

    def ProcessContents(self):
        """Re-read the DTB contents so that we get any calculated properties"""
        _, data = state.GetFdtContents(self._filename)
        self.SetContents(data)
