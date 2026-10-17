# SPDX-License-Identifier: GPL-2.0+
#
# Entry-type module for Rockchip TPL binary
#

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg

class Entry_rockchip_tpl(Entry_blob_named_by_arg):
    """Rockchip TPL binary

    Properties / Entry arguments:
        - rockchip-tpl-path: Filename of file to read into the entry,
                             typically <soc>_ddr_<version>.bin

    This entry holds an external TPL binary used by some Rockchip SoCs
    instead of normal U-Boot TPL, typically to initialize DRAM.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'rockchip-tpl')
        self.external = True
