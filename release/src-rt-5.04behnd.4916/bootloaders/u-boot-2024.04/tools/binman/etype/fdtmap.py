# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>

"""# Entry-type module for a full map of the firmware image

This handles putting an FDT into the image with just the information about the
image.
"""

from binman.entry import Entry
from u_boot_pylib import tools
from u_boot_pylib import tout

FDTMAP_MAGIC   = b'_FDTMAP_'
FDTMAP_HDR_LEN = 16

# These is imported if needed
Fdt = None
libfdt = None
state = None

def LocateFdtmap(data):
    """Search an image for an fdt map

    Args:
        data: Data to search

    Returns:
        Position of fdt map in data, or None if not found. Note that the
            position returned is of the FDT header, i.e. before the FDT data
    """
    hdr_pos = data.find(FDTMAP_MAGIC)
    size = len(data)
    if hdr_pos != -1:
        hdr = data[hdr_pos:hdr_pos + FDTMAP_HDR_LEN]
        if len(hdr) == FDTMAP_HDR_LEN:
            return hdr_pos
    return None

class Entry_fdtmap(Entry):
    """An entry which contains an FDT map

    Properties / Entry arguments:
        None

    An FDT map is just a header followed by an FDT containing a list of all the
    entries in the image. The root node corresponds to the image node in the
    original FDT, and an image-name property indicates the image name in that
    original tree.

    The header is the string _FDTMAP_ followed by 8 unused bytes.

    When used, this entry will be populated with an FDT map which reflects the
    entries in the current image. Hierarchy is preserved, and all offsets and
    sizes are included.

    Note that the -u option must be provided to ensure that binman updates the
    FDT with the position of each entry.

    Example output for a simple image with U-Boot and an FDT map::

        / {
            image-name = "binman";
            size = <0x00000112>;
            image-pos = <0x00000000>;
            offset = <0x00000000>;
            u-boot {
                size = <0x00000004>;
                image-pos = <0x00000000>;
                offset = <0x00000000>;
            };
            fdtmap {
                size = <0x0000010e>;
                image-pos = <0x00000004>;
                offset = <0x00000004>;
            };
        };

    If allow-repack is used then 'orig-offset' and 'orig-size' properties are
    added as necessary. See the binman README.

    When extracting files, an alternative 'fdt' format is available for fdtmaps.
    Use `binman extract -F fdt ...` to use this. It will export a devicetree,
    without the fdtmap header, so it can be viewed with `fdtdump`.
    """
    def __init__(self, section, etype, node):
        # Put these here to allow entry-docs and help to work without libfdt
        global libfdt
        global state
        global Fdt

        import libfdt
        from binman import state
        from dtoc.fdt import Fdt

        super().__init__(section, etype, node)
        self.alt_formats = ['fdt']

    def CheckAltFormats(self, alt_formats):
        alt_formats['fdt'] = self, 'Extract the devicetree blob from the fdtmap'

    def _GetFdtmap(self):
        """Build an FDT map from the entries in the current image

        Returns:
            FDT map binary data
        """
        def _AddNode(node):
            """Add a node to the FDT map"""
            for pname, prop in node.props.items():
                fsw.property(pname, prop.bytes)
            for subnode in node.subnodes:
                with fsw.add_node(subnode.name):
                    _AddNode(subnode)

        data = state.GetFdtContents('fdtmap')[1]
        # If we have an fdtmap it means that we are using this as the
        # fdtmap for this image.
        if data is None:
            # Get the FDT data into an Fdt object
            data = state.GetFdtContents()[1]
            infdt = Fdt.FromData(data)
            infdt.Scan()

            # Find the node for the image containing the Fdt-map entry
            path = self.section.GetPath()
            self.Detail("Fdtmap: Using section '%s' (path '%s')" %
                        (self.section.name, path))
            node = infdt.GetNode(path)
            if not node:
                self.Raise("Internal error: Cannot locate node for path '%s'" %
                           path)

            # Build a new tree with all nodes and properties starting from that
            # node
            fsw = libfdt.FdtSw()
            fsw.finish_reservemap()
            with fsw.add_node(''):
                fsw.property_string('image-node', node.name)
                _AddNode(node)
            fdt = fsw.as_fdt()

            # Pack this new FDT and return its contents
            fdt.pack()
            outfdt = Fdt.FromData(fdt.as_bytearray())
            data = outfdt.GetContents()
        data = FDTMAP_MAGIC + tools.get_bytes(0, 8) + data
        return data

    def ObtainContents(self):
        """Obtain a placeholder for the fdt-map contents"""
        self.SetContents(self._GetFdtmap())
        return True

    def ProcessContents(self):
        """Write an updated version of the FDT map to this entry

        This is necessary since new data may have been written back to it during
        processing, e.g. the image-pos properties.
        """
        return self.ProcessContentsUpdate(self._GetFdtmap())

    def GetAltFormat(self, data, alt_format):
        if alt_format == 'fdt':
            return data[FDTMAP_HDR_LEN:]
