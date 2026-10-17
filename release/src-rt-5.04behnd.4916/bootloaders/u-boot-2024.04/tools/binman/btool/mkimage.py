# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for mkimage"""

import re

from binman import bintool

class Bintoolmkimage(bintool.Bintool):
    """Image generation for U-Boot

    This bintool supports running `mkimage` with some basic parameters as
    neeed by binman.

    Normally binman uses the mkimage built by U-Boot. But when run outside the
    U-Boot build system, binman can use the version installed in your system.
    Support is provided for fetching this on Debian-like systems, using apt.
    """
    def __init__(self, name):
        super().__init__(name, 'Generate image for U-Boot', r'mkimage version (.*)')

    # pylint: disable=R0913
    def run(self, reset_timestamp=False, output_fname=None, external=False,
            pad=None, align=None):
        """Run mkimage

        Args:
            reset_timestamp: True to update the timestamp in the FIT
            output_fname: Output filename to write to
            external: True to create an 'external' FIT, where the binaries are
                located outside the main data structure
            pad: Bytes to use for padding the FIT devicetree output. This allows
                other things to be easily added later, if required, such as
                signatures
            align: Bytes to use for alignment of the FIT and its external data
            version: True to get the mkimage version
        """
        args = []
        if external:
            args.append('-E')
        if pad:
            args += ['-p', f'{pad:x}']
        if align:
            args += ['-B', f'{align:x}']
        if reset_timestamp:
            args.append('-t')
        if output_fname:
            args += ['-F', output_fname]
        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for mkimage

        This installs mkimage using the apt utility.

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
        return self.apt_install('u-boot-tools')
