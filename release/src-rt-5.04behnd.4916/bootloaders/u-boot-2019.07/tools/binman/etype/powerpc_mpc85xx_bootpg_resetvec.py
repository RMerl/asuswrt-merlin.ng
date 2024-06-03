# SPDX-License-Identifier: GPL-2.0+
# Copyright 2018 NXP
#
# Entry-type module for the PowerPC mpc85xx bootpg and resetvec code for U-Boot
#

from entry import Entry
from blob import Entry_blob

class Entry_powerpc_mpc85xx_bootpg_resetvec(Entry_blob):
    """PowerPC mpc85xx bootpg + resetvec code for U-Boot

    Properties / Entry arguments:
        - filename: Filename of u-boot-br.bin (default 'u-boot-br.bin')

    This enrty is valid for PowerPC mpc85xx cpus. This entry holds
    'bootpg + resetvec' code for PowerPC mpc85xx CPUs which needs to be
    placed at offset 'RESET_VECTOR_ADDRESS - 0xffc'.
    """

    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def GetDefaultFilename(self):
        return 'u-boot-br.bin'
