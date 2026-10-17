# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
# Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
#
"""Bintool implementation for zstd

zstd allows compression and decompression of files.

Documentation is available via::

   man zstd
"""

from binman import bintool

# pylint: disable=C0103
class Bintoolzstd(bintool.BintoolPacker):
    """Compression/decompression using the zstd algorithm

    This bintool supports running `zstd` to compress and decompress data, as
    used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man zstd
    """
    def __init__(self, name):
        super().__init__(name)
