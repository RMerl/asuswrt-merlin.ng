# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

# Support for a Chromium OS Google Binary Block, used to record read-only
# information mostly used by firmware.

from collections import OrderedDict

from u_boot_pylib import command
from binman.entry import Entry, EntryArg

from dtoc import fdt_util
from u_boot_pylib import tools

# Build GBB flags.
# (src/platform/vboot_reference/firmware/include/gbb_header.h)
gbb_flag_properties = {
  'dev-screen-short-delay': 0x1,
  'load-option-roms': 0x2,
  'enable-alternate-os': 0x4,
  'force-dev-switch-on': 0x8,
  'force-dev-boot-usb': 0x10,
  'disable-fw-rollback-check': 0x20,
  'enter-triggers-tonorm': 0x40,
  'force-dev-boot-legacy': 0x80,
  'faft-key-override': 0x100,
  'disable-ec-software-sync': 0x200,
  'default-dev-boot-legacy': 0x400,
  'disable-pd-software-sync': 0x800,
  'disable-lid-shutdown': 0x1000,
  'force-dev-boot-fastboot-full-cap': 0x2000,
  'enable-serial': 0x4000,
  'disable-dwmp': 0x8000,
}


class Entry_gbb(Entry):
    """An entry which contains a Chromium OS Google Binary Block

    Properties / Entry arguments:
        - hardware-id: Hardware ID to use for this build (a string)
        - keydir: Directory containing the public keys to use
        - bmpblk: Filename containing images used by recovery

    Chromium OS uses a GBB to store various pieces of information, in particular
    the root and recovery keys that are used to verify the boot process. Some
    more details are here:

        https://www.chromium.org/chromium-os/firmware-porting-guide/2-concepts

    but note that the page dates from 2013 so is quite out of date. See
    README.chromium for how to obtain the required keys and tools.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.hardware_id, self.keydir, self.bmpblk = self.GetEntryArgsOrProps(
            [EntryArg('hardware-id', str),
             EntryArg('keydir', str),
             EntryArg('bmpblk', str)])

        # Read in the GBB flags from the config
        self.gbb_flags = 0
        flags_node = node.FindNode('flags')
        if flags_node:
            for flag, value in gbb_flag_properties.items():
                if fdt_util.GetBool(flags_node, flag):
                    self.gbb_flags |= value

    def ObtainContents(self):
        gbb = 'gbb.bin'
        fname = tools.get_output_filename(gbb)
        if not self.size:
            self.Raise('GBB must have a fixed size')
        gbb_size = self.size
        bmpfv_size = gbb_size - 0x2180
        if bmpfv_size < 0:
            self.Raise('GBB is too small (minimum 0x2180 bytes)')
        keydir = tools.get_input_filename(self.keydir)

        stdout = self.futility.gbb_create(
            fname, [0x100, 0x1000, bmpfv_size, 0x1000])
        if stdout is not None:
            stdout = self.futility.gbb_set(
                fname,
                hwid=self.hardware_id,
                rootkey='%s/root_key.vbpubk' % keydir,
                recoverykey='%s/recovery_key.vbpubk' % keydir,
                flags=self.gbb_flags,
                bmpfv=tools.get_input_filename(self.bmpblk))

        if stdout is not None:
            self.SetContents(tools.read_file(fname))
        else:
            # Bintool is missing; just use the required amount of zero data
            self.record_missing_bintool(self.futility)
            self.SetContents(tools.get_bytes(0, gbb_size))

        return True

    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.futility = self.AddBintool(btools, 'futility')
