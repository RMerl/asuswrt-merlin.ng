# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot ELF image
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_u_boot_elf(Entry_blob):
    """U-Boot ELF image

    Properties / Entry arguments:
        - filename: Filename of u-boot (default 'u-boot')

    This is the U-Boot ELF image. It does not include a device tree but can be
    relocated to any address for execution.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._strip = fdt_util.GetBool(self._node, 'strip')

    def ReadBlobContents(self):
        if self._strip:
            uniq = self.GetUniqueName()
            out_fname = tools.get_output_filename('%s.stripped' % uniq)
            tools.write_file(out_fname, tools.read_file(self._pathname))
            tools.run('strip', out_fname)
            self._pathname = out_fname
        super().ReadBlobContents()
        return True

    def GetDefaultFilename(self):
        return 'u-boot'
