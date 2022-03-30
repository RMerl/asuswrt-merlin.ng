# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a U-Boot binary with an embedded microcode pointer
#

import struct

import command
import elf
from entry import Entry
from blob import Entry_blob
import fdt_util
import tools

class Entry_u_boot_with_ucode_ptr(Entry_blob):
    """U-Boot with embedded microcode pointer

    Properties / Entry arguments:
        - filename: Filename of u-boot-nodtb.dtb (default 'u-boot-nodtb.dtb')
        - optional-ucode: boolean property to make microcode optional. If the
            u-boot.bin image does not include microcode, no error will
            be generated.

    See Entry_u_boot_ucode for full details of the three entries involved in
    this process. This entry updates U-Boot with the offset and size of the
    microcode, to allow early x86 boot code to find it without doing anything
    complicated. Otherwise it is the same as the u_boot entry.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
        self.elf_fname = 'u-boot'
        self.target_offset = None

    def GetDefaultFilename(self):
        return 'u-boot-nodtb.bin'

    def ProcessFdt(self, fdt):
        # Figure out where to put the microcode pointer
        fname = tools.GetInputFilename(self.elf_fname)
        sym = elf.GetSymbolAddress(fname, '_dt_ucode_base_size')
        if sym:
           self.target_offset = sym
        elif not fdt_util.GetBool(self._node, 'optional-ucode'):
            self.Raise('Cannot locate _dt_ucode_base_size symbol in u-boot')
        return True

    def ProcessContents(self):
        # If the image does not need microcode, there is nothing to do
        if not self.target_offset:
            return

        # Get the offset of the microcode
        ucode_entry = self.section.FindEntryType('u-boot-ucode')
        if not ucode_entry:
            self.Raise('Cannot find microcode region u-boot-ucode')

        # Check the target pos is in the section. If it is not, then U-Boot is
        # being linked incorrectly, or is being placed at the wrong offset
        # in the section.
        #
        # The section must be set up so that U-Boot is placed at the
        # flash address to which it is linked. For example, if
        # CONFIG_SYS_TEXT_BASE is 0xfff00000, and the ROM is 8MB, then
        # the U-Boot region must start at offset 7MB in the section. In this
        # case the ROM starts at 0xff800000, so the offset of the first
        # entry in the section corresponds to that.
        if (self.target_offset < self.image_pos or
                self.target_offset >= self.image_pos + self.size):
            self.Raise('Microcode pointer _dt_ucode_base_size at %08x is outside the section ranging from %08x to %08x' %
                (self.target_offset, self.image_pos,
                 self.image_pos + self.size))

        # Get the microcode, either from u-boot-ucode or u-boot-dtb-with-ucode.
        # If we have left the microcode in the device tree, then it will be
        # in the latter. If we extracted the microcode from the device tree
        # and collated it in one place, it will be in the former.
        if ucode_entry.size:
            offset, size = ucode_entry.offset, ucode_entry.size
        else:
            dtb_entry = self.section.FindEntryType('u-boot-dtb-with-ucode')
            if not dtb_entry:
                dtb_entry = self.section.FindEntryType(
                        'u-boot-tpl-dtb-with-ucode')
            if not dtb_entry:
                self.Raise('Cannot find microcode region u-boot-dtb-with-ucode')
            offset = dtb_entry.offset + dtb_entry.ucode_offset
            size = dtb_entry.ucode_size

        # Write the microcode offset and size into the entry
        offset_and_size = struct.pack('<2L', offset, size)
        self.target_offset -= self.image_pos
        self.ProcessContentsUpdate(self.data[:self.target_offset] +
                                   offset_and_size +
                                   self.data[self.target_offset + 8:])
