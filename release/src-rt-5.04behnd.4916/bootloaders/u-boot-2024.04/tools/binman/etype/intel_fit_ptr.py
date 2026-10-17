# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a pointer to an Intel Firmware Image Table
#

import struct

from binman.etype.blob_ext import Entry_blob_ext

class Entry_intel_fit_ptr(Entry_blob_ext):
    """Intel Firmware Image Table (FIT) pointer

    This entry contains a pointer to the FIT. It is required to be at address
    0xffffffc0 in the image.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        if self.HasSibling('intel-fit') is False:
            self.Raise("'intel-fit-ptr' section must have an 'intel-fit' sibling")

    def _GetContents(self):
        fit_pos = self.GetSiblingImagePos('intel-fit')
        return struct.pack('<II', fit_pos or 0, 0)

    def ObtainContents(self):
        self.SetContents(self._GetContents())
        return True

    def ProcessContents(self):
        """Write an updated version of the FIT pointer to this entry

        This is necessary since image_pos is not available when ObtainContents()
        is called, since by then the entries have not been packed in the image.
        """
        return self.ProcessContentsUpdate(self._GetContents())

    def Pack(self, offset):
        """Special pack method to set the offset to the right place"""
        return super().Pack(0xffffffc0)
