# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
# Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
#
"""Bintool implementation for gzip

gzip allows compression and decompression of files.

Documentation is available via::

   man gzip
"""

from binman import bintool

# pylint: disable=C0103
class Bintoolgzip(bintool.BintoolPacker):
    """Compression/decompression using the gzip algorithm

    This bintool supports running `gzip` to compress and decompress data, as
    used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man gzip
    """
    def __init__(self, name):
        super().__init__(name, compress_args=[],
                         version_regex=r'gzip ([0-9.]+)')
