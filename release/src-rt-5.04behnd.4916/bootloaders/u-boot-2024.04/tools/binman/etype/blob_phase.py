# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type base class for U-Boot or SPL binary with devicetree
#

from binman.etype.section import Entry_section

# This is imported if needed
state = None

class Entry_blob_phase(Entry_section):
    """Section that holds a phase binary

    This is a base class that should not normally be used directly. It is used
    when converting a 'u-boot' entry automatically into a 'u-boot-expanded'
    entry; similarly for SPL.
    """
    def __init__(self, section, etype, node, root_fname, dtb_file, bss_pad):
        """Set up a new blob for a phase

        This holds an executable for a U-Boot phase, optional BSS padding and
        a devicetree

        Args:
            section: entry_Section object for this entry's parent
            etype: Type of object
            node: Node defining this entry
            root_fname: Root filename for the binary ('u-boot',
                'spl/u-boot-spl', etc.)
            dtb_file: Name of devicetree file ('u-boot.dtb', u-boot-spl.dtb',
                etc.)
            bss_pad: True to add BSS padding before the devicetree
        """
        # Put this here to allow entry-docs and help to work without libfdt
        global state
        from binman import state

        super().__init__(section, etype, node)
        self.root_fname = root_fname
        self.dtb_file = dtb_file
        self.bss_pad = bss_pad

    def gen_entries(self):
        """Create the subnodes"""
        names = [self.root_fname + '-nodtb', self.root_fname + '-dtb']
        if self.bss_pad:
            names.insert(1, self.root_fname + '-bss-pad')
        for name in names:
            subnode = state.AddSubnode(self._node, name)

        # Read entries again, now that we have some
        self.ReadEntries()

        # Propagate the no-write-symbols property
        if self.no_write_symbols:
            for entry in self._entries.values():
                entry.no_write_symbols = True
