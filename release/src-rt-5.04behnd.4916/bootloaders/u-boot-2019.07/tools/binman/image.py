# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Class for an image, the output of binman
#

from __future__ import print_function

from collections import OrderedDict
from operator import attrgetter
import re
import sys

import fdt_util
import bsection
import tools

class Image:
    """A Image, representing an output from binman

    An image is comprised of a collection of entries each containing binary
    data. The image size must be large enough to hold all of this data.

    This class implements the various operations needed for images.

    Atrtributes:
        _node: Node object that contains the image definition in device tree
        _name: Image name
        _size: Image size in bytes, or None if not known yet
        _filename: Output filename for image
        _sections: Sections present in this image (may be one or more)

    Args:
        test: True if this is being called from a test of Images. This this case
            there is no device tree defining the structure of the section, so
            we create a section manually.
    """
    def __init__(self, name, node, test=False):
        self._node = node
        self._name = name
        self._size = None
        self._filename = '%s.bin' % self._name
        if test:
            self._section = bsection.Section('main-section', None, self._node,
                                             self, True)
        else:
            self._ReadNode()

    def _ReadNode(self):
        """Read properties from the image node"""
        self._size = fdt_util.GetInt(self._node, 'size')
        filename = fdt_util.GetString(self._node, 'filename')
        if filename:
            self._filename = filename
        self._section = bsection.Section('main-section', None, self._node, self)

    def GetFdtSet(self):
        """Get the set of device tree files used by this image"""
        return self._section.GetFdtSet()

    def ExpandEntries(self):
        """Expand out any entries which have calculated sub-entries

        Some entries are expanded out at runtime, e.g. 'files', which produces
        a section containing a list of files. Process these entries so that
        this information is added to the device tree.
        """
        self._section.ExpandEntries()

    def AddMissingProperties(self):
        """Add properties that are not present in the device tree

        When binman has completed packing the entries the offset and size of
        each entry are known. But before this the device tree may not specify
        these. Add any missing properties, with a dummy value, so that the
        size of the entry is correct. That way we can insert the correct values
        later.
        """
        self._section.AddMissingProperties()

    def ProcessFdt(self, fdt):
        """Allow entries to adjust the device tree

        Some entries need to adjust the device tree for their purposes. This
        may involve adding or deleting properties.
        """
        return self._section.ProcessFdt(fdt)

    def GetEntryContents(self):
        """Call ObtainContents() for the section
        """
        self._section.GetEntryContents()

    def GetEntryOffsets(self):
        """Handle entries that want to set the offset/size of other entries

        This calls each entry's GetOffsets() method. If it returns a list
        of entries to update, it updates them.
        """
        self._section.GetEntryOffsets()

    def PackEntries(self):
        """Pack all entries into the image"""
        self._section.PackEntries()

    def CheckSize(self):
        """Check that the image contents does not exceed its size, etc."""
        self._size = self._section.CheckSize()

    def CheckEntries(self):
        """Check that entries do not overlap or extend outside the image"""
        self._section.CheckEntries()

    def SetCalculatedProperties(self):
        self._section.SetCalculatedProperties()

    def SetImagePos(self):
        self._section.SetImagePos(0)

    def ProcessEntryContents(self):
        """Call the ProcessContents() method for each entry

        This is intended to adjust the contents as needed by the entry type.
        """
        self._section.ProcessEntryContents()

    def WriteSymbols(self):
        """Write symbol values into binary files for access at run time"""
        self._section.WriteSymbols()

    def BuildImage(self):
        """Write the image to a file"""
        fname = tools.GetOutputFilename(self._filename)
        with open(fname, 'wb') as fd:
            self._section.BuildSection(fd, 0)

    def GetEntries(self):
        return self._section.GetEntries()

    def WriteMap(self):
        """Write a map of the image to a .map file

        Returns:
            Filename of map file written
        """
        filename = '%s.map' % self._name
        fname = tools.GetOutputFilename(filename)
        with open(fname, 'w') as fd:
            print('%8s  %8s  %8s  %s' % ('ImagePos', 'Offset', 'Size', 'Name'),
                  file=fd)
            self._section.WriteMap(fd, 0)
        return fname
