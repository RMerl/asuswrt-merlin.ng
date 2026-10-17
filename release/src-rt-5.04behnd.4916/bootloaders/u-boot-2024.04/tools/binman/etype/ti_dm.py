# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
# Written by Neha Malcom Francis <n-francis@ti.com>
#
# Entry-type module for TI Device Manager (DM)
#

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg

class Entry_ti_dm(Entry_blob_named_by_arg):
    """TI Device Manager (DM) blob

    Properties / Entry arguments:
        - ti-dm-path: Filename of file to read into the entry, typically ti-dm.bin

    This entry holds the device manager responsible for resource and power management
    in K3 devices. See https://software-dl.ti.com/tisci/esd/latest/ for more information
    about TI DM.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'ti-dm')
        self.external = True
