# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel Firmware Support Package binary blob
#

from binman.etype.blob_ext import Entry_blob_ext

class Entry_intel_fsp(Entry_blob_ext):
    """Intel Firmware Support Package (FSP) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry

    This file contains binary blobs which are used on some devices to make the
    platform work. U-Boot executes this code since it is not possible to set up
    the hardware using U-Boot open-source code. Documentation is typically not
    available in sufficient detail to allow this.

    An example filename is 'FSP/QUEENSBAY_FSP_GOLD_001_20-DECEMBER-2013.fd'

    See README.x86 for information about x86 binary blobs.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
