# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a set of files which are placed in individual
# sub-entries
#

import glob
import os

from section import Entry_section
import fdt_util
import state
import tools

import bsection

class Entry_files(Entry_section):
    """Entry containing a set of files

    Properties / Entry arguments:
        - pattern: Filename pattern to match the files to include
        - compress: Compression algorithm to use:
            none: No compression
            lz4: Use lz4 compression (via 'lz4' command-line utility)

    This entry reads a number of files and places each in a separate sub-entry
    within this entry. To access these you need to enable device-tree updates
    at run-time so you can obtain the file positions.
    """
    def __init__(self, section, etype, node):
        Entry_section.__init__(self, section, etype, node)
        self._pattern = fdt_util.GetString(self._node, 'pattern')
        if not self._pattern:
            self.Raise("Missing 'pattern' property")
        self._compress = fdt_util.GetString(self._node, 'compress', 'none')
        self._require_matches = fdt_util.GetBool(self._node,
                                                'require-matches')

    def ExpandEntries(self):
        files = tools.GetInputFilenameGlob(self._pattern)
        if self._require_matches and not files:
            self.Raise("Pattern '%s' matched no files" % self._pattern)
        for fname in files:
            if not os.path.isfile(fname):
                continue
            name = os.path.basename(fname)
            subnode = self._node.FindNode(name)
            if not subnode:
                subnode = state.AddSubnode(self._node, name)
            state.AddString(subnode, 'type', 'blob')
            state.AddString(subnode, 'filename', fname)
            state.AddString(subnode, 'compress', self._compress)

        # Read entries again, now that we have some
        self._section._ReadEntries()
