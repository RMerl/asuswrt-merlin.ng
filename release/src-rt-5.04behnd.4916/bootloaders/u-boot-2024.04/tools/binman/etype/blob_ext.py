# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for external blobs, not built by U-Boot
#

import os

from binman.etype.blob import Entry_blob
from dtoc import fdt_util
from u_boot_pylib import tools
from u_boot_pylib import tout

class Entry_blob_ext(Entry_blob):
    """Externally built binary blob

    Note: This should not be used by itself. It is normally used as a parent
    class by other entry types.

    If the file providing this blob is missing, binman can optionally ignore it
    and produce a broken image with a warning.

    See 'blob' for Properties / Entry arguments.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
        self.external = True
