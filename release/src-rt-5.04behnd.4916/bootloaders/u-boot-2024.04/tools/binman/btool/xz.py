# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
# Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
#
"""Bintool implementation for xz

xz allows compression and decompression of files.

Documentation is available via::

   man xz
"""

from binman import bintool

# pylint: disable=C0103
class Bintoolxz(bintool.BintoolPacker):
    """Compression/decompression using the xz algorithm

    This bintool supports running `xz` to compress and decompress data, as
    used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man xz
    """
    def __init__(self, name):
        super().__init__(name, fetch_package='xz-utils',
                         version_regex=r'xz \(XZ Utils\) ([0-9.]+)')
