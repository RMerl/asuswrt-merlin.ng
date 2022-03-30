# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot ELF image
#

from entry import Entry
from blob import Entry_blob

import fdt_util
import tools

class Entry_u_boot_elf(Entry_blob):
    """U-Boot ELF image

    Properties / Entry arguments:
        - filename: Filename of u-boot (default 'u-boot')

    This is the U-Boot ELF image. It does not include a device tree but can be
    relocated to any address for execution.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
        self._strip = fdt_util.GetBool(self._node, 'strip')

    def ReadBlobContents(self):
        if self._strip:
            uniq = self.GetUniqueName()
            out_fname = tools.GetOutputFilename('%s.stripped' % uniq)
            tools.WriteFile(out_fname, tools.ReadFile(self._pathname))
            tools.Run('strip', out_fname)
            self._pathname = out_fname
        Entry_blob.ReadBlobContents(self)
        return True

    def GetDefaultFilename(self):
        return 'u-boot'
