# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_fill(Entry):
    """An entry which is filled to a particular byte value

    Properties / Entry arguments:
        - fill-byte: Byte to use to fill the entry

    Note that the size property must be set since otherwise this entry does not
    know how large it should be.

    You can often achieve the same effect using the pad-byte property of the
    overall image, in that the space between entries will then be padded with
    that byte. But this entry is sometimes useful for explicitly setting the
    byte value of a region.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['size']

    def ReadNode(self):
        super().ReadNode()
        self.fill_value = fdt_util.GetByte(self._node, 'fill-byte', 0)

    def ObtainContents(self):
        self.SetContents(tools.get_bytes(self.fill_value, self.size))
        return True
