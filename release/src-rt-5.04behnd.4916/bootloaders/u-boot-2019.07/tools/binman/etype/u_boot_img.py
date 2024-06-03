# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot binary
#

from entry import Entry
from blob import Entry_blob

class Entry_u_boot_img(Entry_blob):
    """U-Boot legacy image

    Properties / Entry arguments:
        - filename: Filename of u-boot.img (default 'u-boot.img')

    This is the U-Boot binary as a packaged image, in legacy format. It has a
    header which allows it to be loaded at the correct address for execution.

    You should use FIT (Flat Image Tree) instead of the legacy image for new
    applications.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def GetDefaultFilename(self):
        return 'u-boot.img'
