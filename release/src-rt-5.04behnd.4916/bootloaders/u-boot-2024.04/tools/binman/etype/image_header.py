# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>

"""Entry-type module for an image header which points to the FDT map

This creates an 8-byte entry with a magic number and the offset of the FDT map
(which is another entry in the image), relative to the start or end of the
image.
"""

import struct

from binman.entry import Entry
from dtoc import fdt_util

IMAGE_HEADER_MAGIC = b'BinM'
IMAGE_HEADER_LEN   = 8

def LocateHeaderOffset(data):
    """Search an image for an image header

    Args:
        data: Data to search

    Returns:
        Offset of image header in the image, or None if not found
    """
    hdr_pos = data.find(IMAGE_HEADER_MAGIC)
    if hdr_pos != -1:
        size = len(data)
        hdr = data[hdr_pos:hdr_pos + IMAGE_HEADER_LEN]
        if len(hdr) == IMAGE_HEADER_LEN:
            offset = struct.unpack('<I', hdr[4:])[0]
            if hdr_pos == len(data) - IMAGE_HEADER_LEN:
                pos = size + offset - (1 << 32)
            else:
                pos = offset
            return pos
    return None

class Entry_image_header(Entry):
    """An entry which contains a pointer to the FDT map

    Properties / Entry arguments:
        location: Location of header ("start" or "end" of image). This is
            optional. If omitted then the entry must have an offset property.

    This adds an 8-byte entry to the start or end of the image, pointing to the
    location of the FDT map. The format is a magic number followed by an offset
    from the start or end of the image, in twos-compliment format.

    This entry must be in the top-level part of the image.

    NOTE: If the location is at the start/end, you will probably need to specify
    sort-by-offset for the image, unless you actually put the image header
    first/last in the entry list.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.location = fdt_util.GetString(self._node, 'location')

    def _GetHeader(self):
        image_pos = self.GetSiblingImagePos('fdtmap')
        if image_pos == False:
            self.Raise("'image_header' section must have an 'fdtmap' sibling")
        elif image_pos is None:
            # This will be available when called from ProcessContents(), but not
            # when called from ObtainContents()
            offset = 0xffffffff
        else:
            image_size = self.section.GetImageSize() or 0
            base = (0 if self.location != 'end' else image_size)
            offset = (image_pos - base) & 0xffffffff
        data = IMAGE_HEADER_MAGIC + struct.pack('<I', offset)
        return data

    def ObtainContents(self):
        """Obtain a placeholder for the header contents"""
        self.SetContents(self._GetHeader())
        return True

    def Pack(self, offset):
        """Special pack method to set the offset to start/end of image"""
        if not self.offset:
            if self.location not in ['start', 'end']:
                self.Raise("Invalid location '%s', expected 'start' or 'end'" %
                           self.location)
            order = self.GetSiblingOrder()
            if self.location != order and not self.section.GetSort():
                self.Raise("Invalid sibling order '%s' for image-header: Must be at '%s' to match location" %
                           (order, self.location))
            if self.location != 'end':
                offset = 0
            else:
                image_size = self.section.GetImageSize()
                if image_size is None:
                    # We don't know the image, but this must be the last entry,
                    # so we can assume it goes
                    offset = offset
                else:
                    offset = image_size - IMAGE_HEADER_LEN
        offset += self.section.GetStartOffset()
        return super().Pack(offset)

    def ProcessContents(self):
        """Write an updated version of the FDT map to this entry

        This is necessary since image_pos is not available when ObtainContents()
        is called, since by then the entries have not been packed in the image.
        """
        return self.ProcessContentsUpdate(self._GetHeader())
