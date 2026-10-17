# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2023 Weidmüller Interface GmbH & Co. KG
# Lukas Funke <lukas.funke@weidmueller.com>
#
"""Bintool implementation for fdt_add_pubkey"""

from binman import bintool

class Bintoolfdt_add_pubkey(bintool.Bintool):
    """Add public key to control dtb (spl or u-boot proper)

    This bintool supports running `fdt_add_pubkey`.

    Normally mkimage adds signature information to the control dtb. However
    binman images are built independent from each other. Thus it is required
    to add the public key separately from mkimage.
    """
    def __init__(self, name):
        super().__init__(name, 'Generate image for U-Boot')

    # pylint: disable=R0913
    def run(self, input_fname, keydir, keyname, required, algo):
        """Run fdt_add_pubkey

        Args:
            input_fname (str): dtb file to sign
            keydir (str): Directory with public key. Optional parameter,
                default value: '.' (current directory)
            keyname (str): Public key name. Optional parameter,
                default value: key
            required (str): If present this indicates that the key must be
                verified for the image / configuration to be considered valid.
            algo (str): Cryptographic algorithm. Optional parameter,
                default value: sha1,rsa2048
        """
        args = []
        if algo:
            args += ['-a', algo]
        if keydir:
            args += ['-k', keydir]
        if keyname:
            args += ['-n', keyname]
        if required:
            args += ['-r', required]

        args += [ input_fname ]

        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for fdt_add_pubkey

        This installs fdt_add_pubkey using the apt utility.

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
