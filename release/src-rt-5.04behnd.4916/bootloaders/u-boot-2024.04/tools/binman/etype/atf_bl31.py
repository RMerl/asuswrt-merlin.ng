# SPDX-License-Identifier: GPL-2.0+
# Copyright 2020 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for ARM Trusted Firmware binary blob
#

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg

class Entry_atf_bl31(Entry_blob_named_by_arg):
    """ARM Trusted Firmware (ATF) BL31 blob

    Properties / Entry arguments:
        - atf-bl31-path: Filename of file to read into entry. This is typically
            called bl31.bin or bl31.elf

    This entry holds the run-time firmware, typically started by U-Boot SPL.
    See the U-Boot README for your architecture or board for how to use it. See
    https://github.com/ARM-software/arm-trusted-firmware for more information
    about ATF.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'atf-bl31')
        self.external = True
