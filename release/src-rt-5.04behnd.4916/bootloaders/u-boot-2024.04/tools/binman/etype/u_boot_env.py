# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

import struct
import zlib

from binman.etype.blob import Entry_blob
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_u_boot_env(Entry_blob):
    """An entry which contains a U-Boot environment

    Properties / Entry arguments:
        - filename: File containing the environment text, with each line in the
            form var=value
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def ReadNode(self):
        super().ReadNode()
        if self.size is None:
            self.Raise("'u-boot-env' entry must have a size property")
        self.fill_value = fdt_util.GetByte(self._node, 'fill-byte', 0)

    def ReadBlobContents(self):
        indata = tools.read_file(self._pathname)
        data = b''
        for line in indata.splitlines():
            data += line + b'\0'
        data += b'\0';
        pad = self.size - len(data) - 5
        if pad < 0:
            self.Raise("'u-boot-env' entry too small to hold data (need %#x more bytes)" % -pad)
        data += tools.get_bytes(self.fill_value, pad)
        crc = zlib.crc32(data)
        buf = struct.pack('<I', crc) + b'\x01' + data
        self.SetContents(buf)
        return True
