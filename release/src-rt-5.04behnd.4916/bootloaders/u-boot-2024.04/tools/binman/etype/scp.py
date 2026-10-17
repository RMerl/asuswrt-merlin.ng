# SPDX-License-Identifier: GPL-2.0+
# Copyright 2020 Samuel Holland <samuel@sholland.org>
#
# Entry-type module for System Control Processor (SCP) firmware blob
#

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg

class Entry_scp(Entry_blob_named_by_arg):
    """System Control Processor (SCP) firmware blob

    Properties / Entry arguments:
        - scp-path: Filename of file to read into the entry, typically scp.bin

    This entry holds firmware for an external platform-specific coprocessor.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'scp')
        self.external = True
