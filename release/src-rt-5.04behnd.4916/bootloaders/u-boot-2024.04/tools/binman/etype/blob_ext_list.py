# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a list of external blobs, not built by U-Boot
#

import os

from binman.etype.blob import Entry_blob
from dtoc import fdt_util
from u_boot_pylib import tools
from u_boot_pylib import tout

class Entry_blob_ext_list(Entry_blob):
    """List of externally built binary blobs

    This is like blob-ext except that a number of blobs can be provided,
    typically with some sort of relationship, e.g. all are DDC parameters.

    If any of the external files needed by this llist is missing, binman can
    optionally ignore it and produce a broken image with a warning.

    Args:
        filenames: List of filenames to read and include
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
        self.external = True

    def ReadNode(self):
        super().ReadNode()
        self._filenames = fdt_util.GetStringList(self._node, 'filenames')
        self._pathnames = []

    def ObtainContents(self):
        missing = False
        pathnames = []
        for fname in self._filenames:
            fname, _ = self.check_fake_fname(fname)
            pathname = tools.get_input_filename(
                fname, self.external and self.section.GetAllowMissing())
            # Allow the file to be missing
            if not pathname:
                missing = True
            pathnames.append(pathname)
        self._pathnames = pathnames

        if missing:
            self.SetContents(b'')
            self.missing = True
            return True

        data = bytearray()
        for pathname in pathnames:
            data += self.ReadFileContents(pathname)

        self.SetContents(data)
        return True
