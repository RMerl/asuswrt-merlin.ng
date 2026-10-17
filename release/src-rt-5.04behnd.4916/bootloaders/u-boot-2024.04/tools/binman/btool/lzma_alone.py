# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for lzma_alone

lzma_alone allows compression and decompression of files, using an older version
of lzma.

Documentation is available via::

   man lzma_alone

Here is the help:

LZMA 9.22 beta : Igor Pavlov : Public domain : 2011-04-18

Usage:  LZMA <e|d> inputFile outputFile [<switches>...]
  e: encode file
  d: decode file
  b: Benchmark
<Switches>
  -a{N}:  set compression mode - [0, 1], default: 1 (max)
  -d{N}:  set dictionary size - [12, 30], default: 23 (8MB)
  -fb{N}: set number of fast bytes - [5, 273], default: 128
  -mc{N}: set number of cycles for match finder
  -lc{N}: set number of literal context bits - [0, 8], default: 3
  -lp{N}: set number of literal pos bits - [0, 4], default: 0
  -pb{N}: set number of pos bits - [0, 4], default: 2
  -mf{MF_ID}: set Match Finder: [bt2, bt3, bt4, hc4], default: bt4
  -mt{N}: set number of CPU threads
  -eos:   write End Of Stream marker
  -si:    read data from stdin
  -so:    write data to stdout
"""

import re
import tempfile

from binman import bintool
from u_boot_pylib import tools

# pylint: disable=C0103
class Bintoollzma_alone(bintool.Bintool):
    """Compression/decompression using the LZMA algorithm

    This bintool supports running `lzma_alone` to compress and decompress data,
    as used by binman.

    It is also possible to fetch the tool, which uses `apt` to install it.

    Documentation is available via::

        man lzma_alone
    """
    def __init__(self, name):
        super().__init__(name, 'lzma_alone compression')

    def compress(self, indata):
        """Compress data with lzma_alone

        Args:
            indata (bytes): Data to compress

        Returns:
            bytes: Compressed data
        """
        with tempfile.NamedTemporaryFile(prefix='comp.tmp',
                                         dir=tools.get_output_dir()) as inf:
            tools.write_file(inf.name, indata)
            with tempfile.NamedTemporaryFile(prefix='compo.otmp',
                                             dir=tools.get_output_dir()) as outf:
                args = ['e', inf.name, outf.name, '-lc1', '-lp0', '-pb0', '-d8']
                self.run_cmd(*args, binary=True)
                return tools.read_file(outf.name)

    def decompress(self, indata):
        """Decompress data with lzma_alone

        Args:
            indata (bytes): Data to decompress

        Returns:
            bytes: Decompressed data
        """
        with tempfile.NamedTemporaryFile(prefix='decomp.tmp',
                                         dir=tools.get_output_dir()) as inf:
            tools.write_file(inf.name, indata)
            with tempfile.NamedTemporaryFile(prefix='compo.otmp',
                                             dir=tools.get_output_dir()) as outf:
                args = ['d', inf.name, outf.name]
                self.run_cmd(*args, binary=True)
                return tools.read_file(outf.name, binary=True)

    def fetch(self, method):
        """Fetch handler for lzma_alone

        This installs the lzma-alone package using the apt utility.

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
        return self.apt_install('lzma-alone')

    def version(self):
        """Version handler

        Returns:
            str: Version number of lzma_alone
        """
        out = self.run_cmd_result('', raise_on_error=False).stderr.strip()
        lines = out.splitlines()
        if not lines:
            return super().version()
        out = lines[0]
        # e.g. LZMA 9.22 beta : Igor Pavlov : Public domain : 2011-04-18
        m_version = re.match(r'LZMA ([^:]*).*', out)
        return m_version.group(1).strip() if m_version else out
