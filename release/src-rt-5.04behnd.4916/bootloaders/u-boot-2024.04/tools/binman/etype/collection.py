# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#

# Support for a collection of entries from other parts of an image

from collections import OrderedDict
import os

from binman.entry import Entry
from dtoc import fdt_util

class Entry_collection(Entry):
    """An entry which contains a collection of other entries

    Properties / Entry arguments:
        - content: List of phandles to entries to include

    This allows reusing the contents of other entries. The contents of the
    listed entries are combined to form this entry. This serves as a useful
    base class for entry types which need to process data from elsewhere in
    the image, not necessarily child entries.

    The entries can generally be anywhere in the same image, even if they are in
    a different section from this entry.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.content = fdt_util.GetPhandleList(self._node, 'content')
        if not self.content:
            self.Raise("Collection must have a 'content' property")

    def GetContents(self, required):
        """Get the contents of this entry

        Args:
            required: True if the data must be present, False if it is OK to
                return None

        Returns:
            bytes content of the entry
        """
        # Join up all the data
        self.Info('Getting contents, required=%s' % required)
        data = bytearray()
        for entry_phandle in self.content:
            entry_data = self.section.GetContentsByPhandle(entry_phandle, self,
                                                           required)
            if not required and entry_data is None:
                self.Info('Contents not available yet')
                # Data not available yet
                return None
            data += entry_data

        self.Info('Returning contents size %x' % len(data))

        return data

    def ObtainContents(self):
        data = self.GetContents(False)
        if data is None:
            return False
        self.SetContents(data)
        return True

    def ProcessContents(self):
        # The blob may have changed due to WriteSymbols()
        data = self.GetContents(True)
        return self.ProcessContentsUpdate(data)
