# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for lz4

lz4 allows compression and decompression of files.

Documentation is available via::

   man lz4

Here is the help:

*** LZ4 command line interface 64-bits v1.9.3, by Yann Collet ***
Usage :
      lz4 [arg] [input] [output]

input   : a filename
          with no FILE, or when FILE is - or stdin, read standard input
Arguments :
 -1     : Fast compression (default)
 -9     : High compression
 -d     : decompression (default for .lz4 extension)
 -z     : force compression
 -D FILE: use FILE as dictionary
 -f     : overwrite output without prompting
 -k     : preserve source files(s)  (default)
--rm    : remove source file(s) after successful de/compression
 -h/-H  : display help/long help and exit

Advanced arguments :
 -V     : display Version number and exit
 -v     : verbose mode
 -q     : suppress warnings; specify twice to suppress errors too
 -c     : force write to standard output, even if it is the console
 -t     : test compressed file integrity
 -m     : multiple input files (implies automatic output filenames)
 -r     : operate recursively on directories (sets also -m)
 -l     : compress using Legacy format (Linux kernel compression)
 -B#    : cut file into blocks of size # bytes [32+]
                     or predefined block size [4-7] (default: 7)
 -BI    : Block Independence (default)
 -BD    : Block dependency (improves compression ratio)
 -BX    : enable block checksum (default:disabled)
--no-frame-crc : disable stream checksum (default:enabled)
--content-size : compressed frame includes original size (default:not present)
--list FILE : lists information about .lz4 files (useful for files compressed
    with --content-size flag)
--[no-]sparse  : sparse mode (default:enabled on file, disabled on stdout)
--favor-decSpeed: compressed files decompress faster, but are less compressed
--fast[=#]: switch to ultra fast compression level (default: 1)
--best  : same as -12
Benchmark arguments :
 -b#    : benchmark file(s), using # compression level (default : 1)
 -e#    : test all compression levels from -bX to # (default : 1)
 -i#    : minimum evaluation time in seconds (default : 3s)
"""

import re
import tempfile

from binman import bintool
from u_boot_pylib import tools

# pylint: disable=C0103
class Bintoollz4(bintool.Bintool):
    """Compression/decompression using the LZ4 algorithm

    This bintool supports running `lz4` to compress and decompress data, as
    used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man lz4
    """
    def __init__(self, name):
        super().__init__(name, 'lz4 compression', r'.* (v[0-9.]*),.*')

    def compress(self, indata):
        """Compress data with lz4

        Args:
            indata (bytes): Data to compress

        Returns:
            bytes: Compressed data
        """
        with tempfile.NamedTemporaryFile(prefix='comp.tmp',
                                         dir=tools.get_output_dir()) as tmp:
            tools.write_file(tmp.name, indata)
            args = ['--no-frame-crc', '-B4', '-5', '-c', tmp.name]
            return self.run_cmd(*args, binary=True)

    def decompress(self, indata):
        """Decompress data with lz4

        Args:
            indata (bytes): Data to decompress

        Returns:
            bytes: Decompressed data
        """
        with tempfile.NamedTemporaryFile(prefix='decomp.tmp',
                                         dir=tools.get_output_dir()) as inf:
            tools.write_file(inf.name, indata)
            args = ['-cd', inf.name]
            return self.run_cmd(*args, binary=True)

    def fetch(self, method):
        """Fetch handler for lz4

        This installs the lz4 package using the apt utility.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched and now installed, None if a method
            other than FETCH_BIN was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        return self.apt_install('lz4')
