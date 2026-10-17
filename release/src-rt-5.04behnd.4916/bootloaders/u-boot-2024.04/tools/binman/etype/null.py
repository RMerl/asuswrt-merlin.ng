# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_null(Entry):
    """An entry which has no contents of its own

    Note that the size property must be set since otherwise this entry does not
    know how large it should be.

    The contents are set by the containing section, e.g. the section's pad
    byte.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['size']

    def ObtainContents(self):
        # null contents
        return None
