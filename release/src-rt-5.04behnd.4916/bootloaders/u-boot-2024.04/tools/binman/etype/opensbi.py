# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>
#
# Entry-type module for RISC-V OpenSBI binary blob
#

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg

class Entry_opensbi(Entry_blob_named_by_arg):
    """RISC-V OpenSBI fw_dynamic blob

    Properties / Entry arguments:
        - opensbi-path: Filename of file to read into entry. This is typically
            called fw_dynamic.bin

    This entry holds the run-time firmware, typically started by U-Boot SPL.
    See the U-Boot README for your architecture or board for how to use it. See
    https://github.com/riscv/opensbi for more information about OpenSBI.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'opensbi')
        self.external = True
