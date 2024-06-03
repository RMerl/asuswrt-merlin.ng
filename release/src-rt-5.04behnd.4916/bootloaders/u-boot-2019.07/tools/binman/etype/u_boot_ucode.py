# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a U-Boot binary with an embedded microcode pointer
#

from entry import Entry
from blob import Entry_blob
import tools

class Entry_u_boot_ucode(Entry_blob):
    """U-Boot microcode block

    Properties / Entry arguments:
        None

    The contents of this entry are filled in automatically by other entries
    which must also be in the image.

    U-Boot on x86 needs a single block of microcode. This is collected from
    the various microcode update nodes in the device tree. It is also unable
    to read the microcode from the device tree on platforms that use FSP
    (Firmware Support Package) binaries, because the API requires that the
    microcode is supplied before there is any SRAM available to use (i.e.
    the FSP sets up the SRAM / cache-as-RAM but does so in the call that
    requires the microcode!). To keep things simple, all x86 platforms handle
    microcode the same way in U-Boot (even non-FSP platforms). This is that
    a table is placed at _dt_ucode_base_size containing the base address and
    size of the microcode. This is either passed to the FSP (for FSP
    platforms), or used to set up the microcode (for non-FSP platforms).
    This all happens in the build system since it is the only way to get
    the microcode into a single blob and accessible without SRAM.

    There are two cases to handle. If there is only one microcode blob in
    the device tree, then the ucode pointer it set to point to that. This
    entry (u-boot-ucode) is empty. If there is more than one update, then
    this entry holds the concatenation of all updates, and the device tree
    entry (u-boot-dtb-with-ucode) is updated to remove the microcode. This
    last step ensures that that the microcode appears in one contiguous
    block in the image and is not unnecessarily duplicated in the device
    tree. It is referred to as 'collation' here.

    Entry types that have a part to play in handling microcode:

        Entry_u_boot_with_ucode_ptr:
            Contains u-boot-nodtb.bin (i.e. U-Boot without the device tree).
            It updates it with the address and size of the microcode so that
            U-Boot can find it early on start-up.
        Entry_u_boot_dtb_with_ucode:
            Contains u-boot.dtb. It stores the microcode in a
            'self.ucode_data' property, which is then read by this class to
            obtain the microcode if needed. If collation is performed, it
            removes the microcode from the device tree.
        Entry_u_boot_ucode:
            This class. If collation is enabled it reads the microcode from
            the Entry_u_boot_dtb_with_ucode entry, and uses it as the
            contents of this entry.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def ObtainContents(self):
        # If the section does not need microcode, there is nothing to do
        found = False
        for suffix in ['', '-spl', '-tpl']:
            name = 'u-boot%s-with-ucode-ptr' % suffix
            entry = self.section.FindEntryType(name)
            if entry and entry.target_offset:
                found = True
        if not found:
            self.data = ''
            return True
        # Get the microcode from the device tree entry. If it is not available
        # yet, return False so we will be called later. If the section simply
        # doesn't exist, then we may as well return True, since we are going to
        # get an error anyway.
        for suffix in ['', '-spl', '-tpl']:
            name = 'u-boot%s-dtb-with-ucode' % suffix
            fdt_entry = self.section.FindEntryType(name)
            if fdt_entry:
                break
        if not fdt_entry:
            return True
        if not fdt_entry.ready:
            return False

        if not fdt_entry.collate:
            # This binary can be empty
            self.data = ''
            return True

        # Write it out to a file
        self._pathname = tools.GetOutputFilename('u-boot-ucode.bin')
        tools.WriteFile(self._pathname, fdt_entry.ucode_data)

        self.ReadBlobContents()

        return True
