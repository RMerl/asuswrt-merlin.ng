# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree with the microcode removed
#

from entry import Entry
from blob_dtb import Entry_blob_dtb
import state
import tools

class Entry_u_boot_dtb_with_ucode(Entry_blob_dtb):
    """A U-Boot device tree file, with the microcode removed

    Properties / Entry arguments:
        - filename: Filename of u-boot.dtb (default 'u-boot.dtb')

    See Entry_u_boot_ucode for full details of the three entries involved in
    this process. This entry provides the U-Boot device-tree file, which
    contains the microcode. If the microcode is not being collated into one
    place then the offset and size of the microcode is recorded by this entry,
    for use by u_boot_with_ucode_ptr. If it is being collated, then this
    entry deletes the microcode from the device tree (to save space) and makes
    it available to u_boot_ucode.
    """
    def __init__(self, section, etype, node):
        Entry_blob_dtb.__init__(self, section, etype, node)
        self.ucode_data = ''
        self.collate = False
        self.ucode_offset = None
        self.ucode_size = None
        self.ucode = None
        self.ready = False

    def GetDefaultFilename(self):
        return 'u-boot.dtb'

    def ProcessFdt(self, fdt):
        # So the module can be loaded without it
        import fdt

        # If the section does not need microcode, there is nothing to do
        ucode_dest_entry = self.section.FindEntryType(
            'u-boot-spl-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_offset:
            ucode_dest_entry = self.section.FindEntryType(
                'u-boot-tpl-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_offset:
            ucode_dest_entry = self.section.FindEntryType(
                'u-boot-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_offset:
            return True

        # Remove the microcode
        fname = self.GetDefaultFilename()
        fdt = state.GetFdt(fname)
        self.ucode = fdt.GetNode('/microcode')
        if not self.ucode:
            raise self.Raise("No /microcode node found in '%s'" % fname)

        # There's no need to collate it (move all microcode into one place)
        # if we only have one chunk of microcode.
        self.collate = len(self.ucode.subnodes) > 1
        for node in self.ucode.subnodes:
            data_prop = node.props.get('data')
            if data_prop:
                self.ucode_data += ''.join(data_prop.bytes)
                if self.collate:
                    node.DeleteProp('data')
        return True

    def ObtainContents(self):
        # Call the base class just in case it does something important.
        Entry_blob_dtb.ObtainContents(self)
        if self.ucode and not self.collate:
            for node in self.ucode.subnodes:
                data_prop = node.props.get('data')
                if data_prop:
                    # Find the offset in the device tree of the ucode data
                    self.ucode_offset = data_prop.GetOffset() + 12
                    self.ucode_size = len(data_prop.bytes)
                    self.ready = True
        else:
            self.ready = True
        return self.ready
