# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
# Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
#
"""Bintool implementation for lzop

lzop allows compression and decompression of files.

Documentation is available via::

   man lzop
"""

from binman import bintool

# pylint: disable=C0103
class Bintoollzop(bintool.BintoolPacker):
    """Compression/decompression using the lzop algorithm

    This bintool supports running `lzop` to compress and decompress data, as
    used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man lzop
    """
    def __init__(self, name):
        super().__init__(name, 'lzo', compress_args=[])
