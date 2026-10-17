# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a Flash map, as used by the flashrom SPI flash tool
#

from binman.entry import Entry
from binman import fmap_util
from u_boot_pylib import tools
from u_boot_pylib.tools import to_hex_size
from u_boot_pylib import tout


class Entry_fmap(Entry):
    """An entry which contains an Fmap section

    Properties / Entry arguments:
        None

    FMAP is a simple format used by flashrom, an open-source utility for
    reading and writing the SPI flash, typically on x86 CPUs. The format
    provides flashrom with a list of areas, so it knows what it in the flash.
    It can then read or write just a single area, instead of the whole flash.

    The format is defined by the flashrom project, in the file lib/fmap.h -
    see www.flashrom.org/Flashrom for more information.

    When used, this entry will be populated with an FMAP which reflects the
    entries in the current image. Note that any hierarchy is squashed, since
    FMAP does not support this. Sections are represented as an area appearing
    before its contents, so that it is possible to reconstruct the hierarchy
    from the FMAP by using the offset information. This convention does not
    seem to be documented, but is used in Chromium OS.

    To mark an area as preserved, use the normal 'preserved' flag in the entry.
    This will result in the corresponding FMAP area having the
    FMAP_AREA_PRESERVE flag. This flag does not automatically propagate down to
    child entries.

    CBFS entries appear as a single entry, i.e. the sub-entries are ignored.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def _GetFmap(self):
        """Build an FMAP from the entries in the current image

        Returns:
            FMAP binary data
        """
        def _AddEntries(areas, entry):
            entries = entry.GetEntries()
            tout.debug("fmap: Add entry '%s' type '%s' (%s subentries)" %
                       (entry.GetPath(), entry.etype, to_hex_size(entries)))

            # Collect any flag (separate lines to ensure code coverage)
            flags = 0
            if entry.preserve:
                flags = fmap_util.FMAP_AREA_PRESERVE

            if entries and entry.etype != 'cbfs':
                # Create an area for the section, which encompasses all entries
                # within it
                if entry.image_pos is None:
                    pos = 0
                else:
                    pos = entry.image_pos - entry.GetRootSkipAtStart()

                # Drop @ symbols in name
                name = entry.name.replace('@', '')
                areas.append(
                    fmap_util.FmapArea(pos, entry.size or 0, name, flags))
                for subentry in entries.values():
                    _AddEntries(areas, subentry)
            else:
                pos = entry.image_pos
                if pos is not None:
                    pos -= entry.section.GetRootSkipAtStart()
                areas.append(fmap_util.FmapArea(pos or 0, entry.size or 0,
                                                entry.name, flags))

        entries = self.GetImage().GetEntries()
        areas = []
        for entry in entries.values():
            _AddEntries(areas, entry)
        return fmap_util.EncodeFmap(self.section.GetImageSize() or 0, self.name,
                                    areas)

    def ObtainContents(self):
        """Obtain a placeholder for the fmap contents"""
        self.SetContents(self._GetFmap())
        return True

    def ProcessContents(self):
        return self.ProcessContentsUpdate(self._GetFmap())
