# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for sections, which are entries which can contain other
# entries.
#

from entry import Entry
import fdt_util
import tools

import bsection

class Entry_section(Entry):
    """Entry that contains other entries

    Properties / Entry arguments: (see binman README for more information)
        - size: Size of section in bytes
        - align-size: Align size to a particular power of two
        - pad-before: Add padding before the entry
        - pad-after: Add padding after the entry
        - pad-byte: Pad byte to use when padding
        - sort-by-offset: Reorder the entries by offset
        - end-at-4gb: Used to build an x86 ROM which ends at 4GB (2^32)
        - name-prefix: Adds a prefix to the name of every entry in the section
            when writing out the map

    A section is an entry which can contain other entries, thus allowing
    hierarchical images to be created. See 'Sections and hierarchical images'
    in the binman README for more information.
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self._section = bsection.Section(node.name, section, node,
                                         section._image)

    def GetFdtSet(self):
        return self._section.GetFdtSet()

    def ProcessFdt(self, fdt):
        return self._section.ProcessFdt(fdt)

    def ExpandEntries(self):
        Entry.ExpandEntries(self)
        self._section.ExpandEntries()

    def AddMissingProperties(self):
        Entry.AddMissingProperties(self)
        self._section.AddMissingProperties()

    def ObtainContents(self):
        return self._section.GetEntryContents()

    def GetData(self):
        return self._section.GetData()

    def GetOffsets(self):
        """Handle entries that want to set the offset/size of other entries

        This calls each entry's GetOffsets() method. If it returns a list
        of entries to update, it updates them.
        """
        self._section.GetEntryOffsets()
        return {}

    def Pack(self, offset):
        """Pack all entries into the section"""
        self._section.PackEntries()
        if self._section._offset is None:
            self._section.SetOffset(offset)
        self.size = self._section.GetSize()
        return super(Entry_section, self).Pack(offset)

    def SetImagePos(self, image_pos):
        Entry.SetImagePos(self, image_pos)
        self._section.SetImagePos(image_pos + self.offset)

    def WriteSymbols(self, section):
        """Write symbol values into binary files for access at run time"""
        self._section.WriteSymbols()

    def SetCalculatedProperties(self):
        Entry.SetCalculatedProperties(self)
        self._section.SetCalculatedProperties()

    def ProcessContents(self):
        self._section.ProcessEntryContents()
        super(Entry_section, self).ProcessContents()

    def CheckOffset(self):
        self._section.CheckEntries()

    def WriteMap(self, fd, indent):
        """Write a map of the section to a .map file

        Args:
            fd: File to write the map to
        """
        self._section.WriteMap(fd, indent)

    def GetEntries(self):
        return self._section.GetEntries()

    def ExpandToLimit(self, limit):
        super(Entry_section, self).ExpandToLimit(limit)
        self._section.ExpandSize(self.size)
