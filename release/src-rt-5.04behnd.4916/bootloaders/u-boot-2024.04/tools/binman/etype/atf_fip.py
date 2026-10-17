# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for the ARM Trusted Firmware's Firmware Image Package (FIP)
# format

from collections import OrderedDict

from binman.entry import Entry
from binman.etype.section import Entry_section
from binman.fip_util import FIP_TYPES, FipReader, FipWriter, UUID_LEN
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_atf_fip(Entry_section):
    """ARM Trusted Firmware's Firmware Image Package (FIP)

    A FIP_ provides a way to group binaries in a firmware image, used by ARM's
    Trusted Firmware A (TF-A) code. It is a simple format consisting of a
    table of contents with information about the type, offset and size of the
    binaries in the FIP. It is quite similar to FMAP, with the major difference
    that it uses UUIDs to indicate the type of each entry.

    Note: It is recommended to always add an fdtmap to every image, as well as
    any FIPs so that binman and other tools can access the entire image
    correctly.

    The UUIDs correspond to useful names in `fiptool`, provided by ATF to
    operate on FIPs. Binman uses these names to make it easier to understand
    what is going on, although it is possible to provide a UUID if needed.

    The contents of the FIP are defined by subnodes of the atf-fip entry, e.g.::

        atf-fip {
            soc-fw {
                filename = "bl31.bin";
            };

            scp-fwu-cfg {
                filename = "bl2u.bin";
            };

            u-boot {
                fip-type = "nt-fw";
            };
        };

    This describes a FIP with three entries: soc-fw, scp-fwu-cfg and nt-fw.
    You can use normal (non-external) binaries like U-Boot simply by adding a
    FIP type, with the `fip-type` property, as above.

    Since FIP exists to bring blobs together, Binman assumes that all FIP
    entries are external binaries. If a binary may not exist, you can use the
    `--allow-missing` flag to Binman, in which case the image is still created,
    even though it will not actually work.

    The size of the FIP depends on the size of the binaries. There is currently
    no way to specify a fixed size. If the `atf-fip` node has a `size` entry,
    this affects the space taken up by the `atf-fip` entry, but the FIP itself
    does not expand to use that space.

    Some other FIP features are available with Binman. The header and the
    entries have 64-bit flag works. The flag flags do not seem to be defined
    anywhere, but you can use `fip-hdr-flags` and fip-flags` to set the values
    of the header and entries respectively.

    FIP entries can be aligned to a particular power-of-two boundary. Use
    fip-align for this.

    Binman only understands the entry types that are included in its
    implementation. It is possible to specify a 16-byte UUID instead, using the
    fip-uuid property. In this case Binman doesn't know what its type is, so
    just uses the UUID. See the `u-boot` node in this example::

        binman {
            atf-fip {
                fip-hdr-flags = /bits/ 64 <0x123>;
                fip-align = <16>;
                soc-fw {
                    fip-flags = /bits/ 64 <0x456>;
                    filename = "bl31.bin";
                };

                scp-fwu-cfg {
                    filename = "bl2u.bin";
                };

                u-boot {
                    fip-uuid = [fc 65 13 92 4a 5b 11 ec
                                94 35 ff 2d 1c fc 79 9c];
                };
            };
            fdtmap {
            };
        };

    Binman allows reading and updating FIP entries after the image is created,
    provided that an FDPMAP is present too. Updates which change the size of a
    FIP entry will cause it to be expanded or contracted as needed.

    Properties for top-level atf-fip node
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    fip-hdr-flags (64 bits)
        Sets the flags for the FIP header.

    Properties for subnodes
    ~~~~~~~~~~~~~~~~~~~~~~~

    fip-type (str)
        FIP type to use for this entry. This is needed if the entry
        name is not a valid type. Value types are defined in `fip_util.py`.
        The FIP type defines the UUID that is used (they map 1:1).

    fip-uuid (16 bytes)
        If there is no FIP-type name defined, or it is not supported by Binman,
        this property sets the UUID. It should be a 16-byte value, following the
        hex digits of the UUID.

    fip-flags (64 bits)
        Set the flags for a FIP entry. Use in one of the subnodes of the
        7atf-fip entry.

    fip-align
        Set the alignment for a FIP entry, FIP entries can be aligned to a
        particular power-of-two boundary. The default is 1.

    Adding new FIP-entry types
    ~~~~~~~~~~~~~~~~~~~~~~~~~~

    When new FIP entries are defined by TF-A they appear in the
    `TF-A source tree`_. You can use `fip_util.py` to update Binman to support
    new types, then `send a patch`_ to the U-Boot mailing list. There are two
    source files that the tool examples:

    - `include/tools_share/firmware_image_package.h` has the UUIDs
    - `tools/fiptool/tbbr_config.c` has the name and descripion for each UUID

    To run the tool::

        $ tools/binman/fip_util.py  -s /path/to/arm-trusted-firmware
        Warning: UUID 'UUID_NON_TRUSTED_WORLD_KEY_CERT' is not mentioned in tbbr_config.c file
        Existing code in 'tools/binman/fip_util.py' is up-to-date

    If it shows there is an update, it writes a new version of `fip_util.py`
    to `fip_util.py.out`. You can change the output file using the `-i` flag.
    If you have a problem, use `-D` to enable traceback debugging.

    FIP commentary
    ~~~~~~~~~~~~~~

    As a side effect of use of UUIDs, FIP does not support multiple
    entries of the same type, such as might be used to store fonts or graphics
    icons, for example. For verified boot it could be used for each part of the
    image (e.g. separate FIPs for A and B) but cannot describe the whole
    firmware image. As with FMAP there is no hierarchy defined, although FMAP
    works around this by having 'section' areas which encompass others. A
    similar workaround would be possible with FIP but is not currently defined.

    It is recommended to always add an fdtmap to every image, as well as any
    FIPs so that binman and other tools can access the entire image correctly.

    .. _FIP: https://trustedfirmware-a.readthedocs.io/en/latest/design/firmware-design.html#firmware-image-package-fip
    .. _`TF-A source tree`: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    .. _`send a patch`: https://www.denx.de/wiki/U-Boot/Patches
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
        self._pad_byte = fdt_util.GetInt(self._node, 'pad-byte', 0)
        self._fip_flags = fdt_util.GetInt64(self._node, 'fip-hdr-flags', 0)
        self._fip_align = fdt_util.GetInt(self._node, 'fip-align', 1)
        if tools.not_power_of_two(self._fip_align):
            raise ValueError("Node '%s': FIP alignment %s must be a power of two" %
                             (self._node.path, self._fip_align))
        self.ReadEntries()

    def ReadEntries(self):
        """Read the subnodes to find out what should go in this FIP"""
        for node in self._node.subnodes:
            fip_type = None
            etype = None
            if node.name in FIP_TYPES:
                fip_type = node.name
                etype = 'blob-ext'

            entry = Entry.Create(self, node, etype)
            entry._fip_uuid = fdt_util.GetBytes(node, 'fip-uuid', UUID_LEN)
            if not fip_type and not entry._fip_uuid:
                fip_type = fdt_util.GetString(node, 'fip-type')
                if not fip_type:
                    self.Raise("Must provide a fip-type (node name '%s' is not a known FIP type)" %
                               node.name)

            entry._fip_type = fip_type
            entry._fip_flags = fdt_util.GetInt64(node, 'fip-flags', 0)
            entry.ReadNode()
            entry._fip_name = node.name
            self._entries[entry._fip_name] = entry

    def BuildSectionData(self, required):
        """Override this function to create a custom format for the entries

        Arguments:
            required (bool): True if the data must be valid, False if it may
                be missing (entry.GetData() returns None

        Returns:
            bytes: Data obtained, or None if None
        """
        fip = FipWriter(self._fip_flags, self._fip_align)
        for entry in self._entries.values():
            # First get the input data and put it in an entry. If not available,
            # try later.
            entry_data = entry.GetData(required)
            if not required and entry_data is None:
                return None
            fent = fip.add_entry(entry._fip_type or entry._fip_uuid, entry_data,
                                 entry._fip_flags)
            if fent:
                entry._fip_entry = fent
        data = fip.get_data()
        return data

    def SetImagePos(self, image_pos):
        """Override this function to set all the entry properties from FIP

        We can only do this once image_pos is known

        Args:
            image_pos: Position of this entry in the image
        """
        super().SetImagePos(image_pos)

        # Now update the entries with info from the FIP entries
        for entry in self._entries.values():
            fent = entry._fip_entry
            entry.size = fent.size
            entry.offset = fent.offset
            entry.image_pos = self.image_pos + entry.offset

    def ReadChildData(self, child, decomp=True, alt_format=None):
        if not self.reader:
            self.fip_data = super().ReadData(True)
            self.reader = FipReader(self.fip_data)
        reader = self.reader

        # It is tricky to obtain the data from a FIP entry since it is indexed
        # by its UUID.
        fent = reader.get_entry(child._fip_type or child._fip_uuid)
        return fent.data

        # Note:
        # It is also possible to extract it using the offsets directly, but this
        # seems less FIP_friendly:
        # return self.fip_data[child.offset:child.offset + child.size]

    def WriteChildData(self, child):
        # Recreate the data structure, leaving the data for this child alone,
        # so that child.data is used to pack into the FIP.
        self.ObtainContents(skip_entry=child)
        return super().WriteChildData(child)
