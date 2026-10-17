# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel flash descriptor
#

import struct

from binman.entry import Entry
from binman.etype.blob_ext import Entry_blob_ext

FD_SIGNATURE   = struct.pack('<L', 0x0ff0a55a)
MAX_REGIONS    = 5

# Region numbers supported by the Intel firmware format
(REGION_DESCRIPTOR, REGION_BIOS, REGION_ME, REGION_GBE,
        REGION_PDATA) = range(5)

class Region:
    def __init__(self, data, frba, region_num):
        pos = frba + region_num * 4
        val = struct.unpack('<L', data[pos:pos + 4])[0]
        self.base = (val & 0xfff) << 12
        self.limit = ((val & 0x0fff0000) >> 4) | 0xfff
        self.size = self.limit - self.base + 1

class Entry_intel_descriptor(Entry_blob_ext):
    """Intel flash descriptor block (4KB)

    Properties / Entry arguments:
        filename: Filename of file containing the descriptor. This is typically
            a 4KB binary file, sometimes called 'descriptor.bin'

    This entry is placed at the start of flash and provides information about
    the SPI flash regions. In particular it provides the base address and
    size of the ME (Management Engine) region, allowing us to place the ME
    binary in the right place.

    With this entry in your image, the position of the 'intel-me' entry will be
    fixed in the image, which avoids you needed to specify an offset for that
    region. This is useful, because it is not possible to change the position
    of the ME region without updating the descriptor.

    See README.x86 for information about x86 binary blobs.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._regions = []

    def Pack(self, offset):
        """Put this entry at the start of the image"""
        if self.offset is None:
            offset = self.section.GetStartOffset()
        return super().Pack(offset)

    def GetOffsets(self):
        info = {}
        if self.missing:
            # Return zero offsets so that these entries get placed somewhere
            if self.HasSibling('intel-me'):
                info['intel-me'] = [0, None]
            return info
        offset = self.data.find(FD_SIGNATURE)
        if offset == -1:
            self.Raise('Cannot find Intel Flash Descriptor (FD) signature')
        flvalsig, flmap0, flmap1, flmap2 = struct.unpack('<LLLL',
                                                self.data[offset:offset + 16])
        frba = ((flmap0 >> 16) & 0xff) << 4
        for i in range(MAX_REGIONS):
            self._regions.append(Region(self.data, frba, i))

        # Set the offset for ME (Management Engine) and IFWI (Integrated
        # Firmware Image), for now, since the others are not used.
        if self.HasSibling('intel-me'):
            info['intel-me'] = [self._regions[REGION_ME].base,
                                self._regions[REGION_ME].size]
        if self.HasSibling('intel-ifwi'):
            info['intel-ifwi'] = [self._regions[REGION_BIOS].base, None]
        return info
