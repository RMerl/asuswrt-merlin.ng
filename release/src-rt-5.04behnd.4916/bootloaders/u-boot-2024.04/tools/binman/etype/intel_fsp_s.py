# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel Firmware Support Package binary blob (S section)
#

from binman.etype.blob_ext import Entry_blob_ext

class Entry_intel_fsp_s(Entry_blob_ext):
    """Intel Firmware Support Package (FSP) silicon init

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains a binary blob which is used on some devices to set up
    the silicon. U-Boot executes this code in U-Boot proper after SDRAM is
    running, so that it can make full use of memory. Documentation is typically
    not available in sufficient detail to allow U-Boot do this this itself.

    An example filename is 'fsp_s.bin'

    See README.x86 for information about x86 binary blobs.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
