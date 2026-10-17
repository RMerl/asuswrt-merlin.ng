# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
# Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
#
"""Bintool implementation for bzip2

bzip2 allows compression and decompression of files.

Documentation is available via::

   man bzip2
"""

from binman import bintool

# pylint: disable=C0103
class Bintoolbzip2(bintool.BintoolPacker):
    """Compression/decompression using the bzip2 algorithm

    This bintool supports running `bzip2` to compress and decompress data, as
    used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man bzip2
    """
    def __init__(self, name):
        super().__init__(name, version_regex=r'bzip2.*Version ([0-9.]+)', version_args='--help')
