# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for blobs, which are binary objects read from files
#

from entry import Entry
import fdt_util
import state
import tools

class Entry_blob(Entry):
    """Entry containing an arbitrary binary blob

    Note: This should not be used by itself. It is normally used as a parent
    class by other entry types.

    Properties / Entry arguments:
        - filename: Filename of file to read into entry
        - compress: Compression algorithm to use:
            none: No compression
            lz4: Use lz4 compression (via 'lz4' command-line utility)

    This entry reads data from a file and places it in the entry. The
    default filename is often specified specified by the subclass. See for
    example the 'u_boot' entry which provides the filename 'u-boot.bin'.

    If compression is enabled, an extra 'uncomp-size' property is written to
    the node (if enabled with -u) which provides the uncompressed size of the
    data.
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self._filename = fdt_util.GetString(self._node, 'filename', self.etype)
        self._compress = fdt_util.GetString(self._node, 'compress', 'none')
        self._uncompressed_size = None

    def ObtainContents(self):
        self._filename = self.GetDefaultFilename()
        self._pathname = tools.GetInputFilename(self._filename)
        self.ReadBlobContents()
        return True

    def ReadBlobContents(self):
        # We assume the data is small enough to fit into memory. If this
        # is used for large filesystem image that might not be true.
        # In that case, Image.BuildImage() could be adjusted to use a
        # new Entry method which can read in chunks. Then we could copy
        # the data in chunks and avoid reading it all at once. For now
        # this seems like an unnecessary complication.
        data = tools.ReadFile(self._pathname)
        if self._compress == 'lz4':
            self._uncompressed_size = len(data)
            '''
            import lz4  # Import this only if needed (python-lz4 dependency)

            try:
                data = lz4.frame.compress(data)
            except AttributeError:
                data = lz4.compress(data)
            '''
            data = tools.Run('lz4', '-c', self._pathname)
        self.SetContents(data)
        return True

    def GetDefaultFilename(self):
        return self._filename

    def AddMissingProperties(self):
        Entry.AddMissingProperties(self)
        if self._compress != 'none':
            state.AddZeroProp(self._node, 'uncomp-size')

    def SetCalculatedProperties(self):
        Entry.SetCalculatedProperties(self)
        if self._uncompressed_size is not None:
            state.SetInt(self._node, 'uncomp-size', self._uncompressed_size)
