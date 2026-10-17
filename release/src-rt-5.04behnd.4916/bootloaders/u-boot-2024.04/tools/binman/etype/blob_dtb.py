# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree files
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob
from dtoc import fdt_util
import struct

# This is imported if needed
state = None

class Entry_blob_dtb(Entry_blob):
    """A blob that holds a device tree

    This is a blob containing a device tree. The contents of the blob are
    obtained from the list of available device-tree files, managed by the
    'state' module.

    Additional attributes:
        prepend: Header used (e.g. 'length')
    """
    def __init__(self, section, etype, node):
        # Put this here to allow entry-docs and help to work without libfdt
        global state
        from binman import state

        super().__init__(section, etype, node)
        self.prepend = None

    def ReadNode(self):
        super().ReadNode()
        self.prepend = fdt_util.GetString(self._node, 'prepend')
        if self.prepend and self.prepend not in ['length']:
            self.Raise("Invalid prepend in '%s': '%s'" %
                       (self._node.name, self.prepend))

    def ObtainContents(self, fake_size=0):
        """Get the device-tree from the list held by the 'state' module"""
        self._filename = self.GetDefaultFilename()
        self._pathname, _ = state.GetFdtContents(self.GetFdtEtype())
        return super().ReadBlobContents()

    def ProcessContents(self):
        """Re-read the DTB contents so that we get any calculated properties"""
        _, indata = state.GetFdtContents(self.GetFdtEtype())

        if self.compress == 'zstd' and self.prepend != 'length':
            self.Raise('The zstd compression requires a length header')

        data = self.CompressData(indata)
        return self.ProcessContentsUpdate(data)

    def GetFdtEtype(self):
        """Get the entry type of this device tree

        This can be 'u-boot-dtb', 'u-boot-spl-dtb' or 'u-boot-tpl-dtb'
        Returns:
            Entry type if any, e.g. 'u-boot-dtb'
        """
        return None

    def GetFdts(self):
        fname = self.GetDefaultFilename()
        return {self.GetFdtEtype(): [self, fname]}

    def WriteData(self, data, decomp=True):
        ok = super().WriteData(data, decomp)

        # Update the state module, since it has the authoritative record of the
        # device trees used. If we don't do this, then state.GetFdtContents()
        # will still return the old contents
        state.UpdateFdtContents(self.GetFdtEtype(), data)
        return ok

    def CompressData(self, indata):
        data = super().CompressData(indata)
        if self.prepend == 'length':
            hdr = struct.pack('<I', len(data))
            data = hdr + data
        return data

    def DecompressData(self, indata):
        if self.prepend == 'length':
            data_len = struct.unpack('<I', indata[:4])[0]
            indata = indata[4:4 + data_len]
        data = super().DecompressData(indata)
        return data
