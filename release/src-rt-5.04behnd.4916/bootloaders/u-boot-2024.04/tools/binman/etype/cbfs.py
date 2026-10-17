# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a Coreboot Filesystem (CBFS)
#

from collections import OrderedDict

from binman import cbfs_util
from binman.cbfs_util import CbfsWriter
from binman.entry import Entry
from dtoc import fdt_util

# This is imported if needed
state = None

class Entry_cbfs(Entry):
    """Coreboot Filesystem (CBFS)

    A CBFS provides a way to group files into a group. It has a simple directory
    structure and allows the position of individual files to be set, since it is
    designed to support execute-in-place in an x86 SPI-flash device. Where XIP
    is not used, it supports compression and storing ELF files.

    CBFS is used by coreboot as its way of orgnanising SPI-flash contents.

    The contents of the CBFS are defined by subnodes of the cbfs entry, e.g.::

        cbfs {
            size = <0x100000>;
            u-boot {
                cbfs-type = "raw";
            };
            u-boot-dtb {
                cbfs-type = "raw";
            };
        };

    This creates a CBFS 1MB in size two files in it: u-boot.bin and u-boot.dtb.
    Note that the size is required since binman does not support calculating it.
    The contents of each entry is just what binman would normally provide if it
    were not a CBFS node. A blob type can be used to import arbitrary files as
    with the second subnode below::

        cbfs {
            size = <0x100000>;
            u-boot {
                cbfs-name = "BOOT";
                cbfs-type = "raw";
            };

            dtb {
                type = "blob";
                filename = "u-boot.dtb";
                cbfs-type = "raw";
                cbfs-compress = "lz4";
                cbfs-offset = <0x100000>;
            };
        };

    This creates a CBFS 1MB in size with u-boot.bin (named "BOOT") and
    u-boot.dtb (named "dtb") and compressed with the lz4 algorithm.


    Properties supported in the top-level CBFS node:

    cbfs-arch:
        Defaults to "x86", but you can specify the architecture if needed.


    Properties supported in the CBFS entry subnodes:

    cbfs-name:
        This is the name of the file created in CBFS. It defaults to the entry
        name (which is the node name), but you can override it with this
        property.

    cbfs-type:
        This is the CBFS file type. The following are supported:

        raw:
            This is a 'raw' file, although compression is supported. It can be
            used to store any file in CBFS.

        stage:
            This is an ELF file that has been loaded (i.e. mapped to memory), so
            appears in the CBFS as a flat binary. The input file must be an ELF
            image, for example this puts "u-boot" (the ELF image) into a 'stage'
            entry::

                cbfs {
                    size = <0x100000>;
                    u-boot-elf {
                        cbfs-name = "BOOT";
                        cbfs-type = "stage";
                    };
                };

            You can use your own ELF file with something like::

                cbfs {
                    size = <0x100000>;
                    something {
                        type = "blob";
                        filename = "cbfs-stage.elf";
                        cbfs-type = "stage";
                    };
                };

            As mentioned, the file is converted to a flat binary, so it is
            equivalent to adding "u-boot.bin", for example, but with the load and
            start addresses specified by the ELF. At present there is no option
            to add a flat binary with a load/start address, similar to the
            'add-flat-binary' option in cbfstool.

    cbfs-offset:
        This is the offset of the file's data within the CBFS. It is used to
        specify where the file should be placed in cases where a fixed position
        is needed. Typical uses are for code which is not relocatable and must
        execute in-place from a particular address. This works because SPI flash
        is generally mapped into memory on x86 devices. The file header is
        placed before this offset so that the data start lines up exactly with
        the chosen offset. If this property is not provided, then the file is
        placed in the next available spot.

    The current implementation supports only a subset of CBFS features. It does
    not support other file types (e.g. payload), adding multiple files (like the
    'files' entry with a pattern supported by binman), putting files at a
    particular offset in the CBFS and a few other things.

    Of course binman can create images containing multiple CBFSs, simply by
    defining these in the binman config::


        binman {
            size = <0x800000>;
            cbfs {
                offset = <0x100000>;
                size = <0x100000>;
                u-boot {
                    cbfs-type = "raw";
                };
                u-boot-dtb {
                    cbfs-type = "raw";
                };
            };

            cbfs2 {
                offset = <0x700000>;
                size = <0x100000>;
                u-boot {
                    cbfs-type = "raw";
                };
                u-boot-dtb {
                    cbfs-type = "raw";
                };
                image {
                    type = "blob";
                    filename = "image.jpg";
                };
            };
        };

    This creates an 8MB image with two CBFSs, one at offset 1MB, one at 7MB,
    both of size 1MB.
    """
    def __init__(self, section, etype, node):
        # Put this here to allow entry-docs and help to work without libfdt
        global state
        from binman import state

        super().__init__(section, etype, node)
        self.align_default = None
        self._entries = OrderedDict()
        self.reader = None

    def ReadNode(self):
        """Read properties from the atf-fip node"""
        super().ReadNode()
        self._cbfs_arg = fdt_util.GetString(self._node, 'cbfs-arch', 'x86')
        self.ReadEntries()

    def ReadEntries(self):
        """Read the subnodes to find out what should go in this CBFS"""
        for node in self._node.subnodes:
            entry = Entry.Create(self, node)
            entry.ReadNode()
            entry._cbfs_name = fdt_util.GetString(node, 'cbfs-name', entry.name)
            entry._type = fdt_util.GetString(node, 'cbfs-type')
            compress = fdt_util.GetString(node, 'cbfs-compress', 'none')
            entry._cbfs_offset = fdt_util.GetInt(node, 'cbfs-offset')
            entry._cbfs_compress = cbfs_util.find_compress(compress)
            if entry._cbfs_compress is None:
                self.Raise("Invalid compression in '%s': '%s'" %
                           (node.name, compress))
            self._entries[entry._cbfs_name] = entry

    def ObtainCfile(self, cbfs, entry):
        # First get the input data and put it in a file. If not available,
        # try later.
        data = entry.GetData()
        cfile = None
        if entry._type == 'raw':
            cfile = cbfs.add_file_raw(entry._cbfs_name, data,
                                      entry._cbfs_offset,
                                      entry._cbfs_compress)
        elif entry._type == 'stage':
            cfile = cbfs.add_file_stage(entry._cbfs_name, data,
                                        entry._cbfs_offset)
        else:
            entry.Raise("Unknown cbfs-type '%s' (use 'raw', 'stage')" %
                        entry._type)
        return cfile

    def ObtainContents(self, skip_entry=None):
        arch = cbfs_util.find_arch(self._cbfs_arg)
        if arch is None:
            self.Raise("Invalid architecture '%s'" % self._cbfs_arg)
        if self.size is None:
            self.Raise("'cbfs' entry must have a size property")
        cbfs = CbfsWriter(self.size, arch)
        for entry in self._entries.values():
            if entry != skip_entry and not entry.ObtainContents():
                return False
            cfile = self.ObtainCfile(cbfs, entry)
            if cfile:
                entry._cbfs_file = cfile
        data = cbfs.get_data()
        self.SetContents(data)
        return True

    def SetImagePos(self, image_pos):
        """Override this function to set all the entry properties from CBFS

        We can only do this once image_pos is known

        Args:
            image_pos: Position of this entry in the image
        """
        super().SetImagePos(image_pos)

        # Now update the entries with info from the CBFS entries
        for entry in self._entries.values():
            cfile = entry._cbfs_file
            entry.size = cfile.data_len
            entry.offset = cfile.calced_cbfs_offset
            entry.image_pos = self.image_pos + entry.offset
            if entry._cbfs_compress:
                entry.uncomp_size = cfile.memlen

    def AddMissingProperties(self, have_image_pos):
        super().AddMissingProperties(have_image_pos)
        for entry in self._entries.values():
            entry.AddMissingProperties(have_image_pos)
            if entry._cbfs_compress:
                state.AddZeroProp(entry._node, 'uncomp-size')
                # Store the 'compress' property, since we don't look at
                # 'cbfs-compress' in Entry.ReadData()
                state.AddString(entry._node, 'compress',
                                cbfs_util.compress_name(entry._cbfs_compress))

    def SetCalculatedProperties(self):
        """Set the value of device-tree properties calculated by binman"""
        super().SetCalculatedProperties()
        for entry in self._entries.values():
            state.SetInt(entry._node, 'offset', entry.offset)
            state.SetInt(entry._node, 'size', entry.size)
            state.SetInt(entry._node, 'image-pos', entry.image_pos)
            if entry.uncomp_size is not None:
                state.SetInt(entry._node, 'uncomp-size', entry.uncomp_size)

    def ListEntries(self, entries, indent):
        """Override this method to list all files in the section"""
        super().ListEntries(entries, indent)
        for entry in self._entries.values():
            entry.ListEntries(entries, indent + 1)

    def GetEntries(self):
        return self._entries

    def ReadData(self, decomp=True, alt_format=None):
        data = super().ReadData(True, alt_format)
        return data

    def ReadChildData(self, child, decomp=True, alt_format=None):
        if not self.reader:
            data = super().ReadData(True, alt_format)
            self.reader = cbfs_util.CbfsReader(data)
        reader = self.reader
        cfile = reader.files.get(child.name)
        return cfile.data if decomp else cfile.orig_data

    def WriteChildData(self, child):
        # Recreate the data structure, leaving the data for this child alone,
        # so that child.data is used to pack into the FIP.
        self.ObtainContents(skip_entry=child)
        return super().WriteChildData(child)

    def AddBintools(self, btools):
        super().AddBintools(btools)
        for entry in self._entries.values():
            entry.AddBintools(btools)
