# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Weidmueller GmbH
# Written by Lukas Funke <lukas.funke@weidmueller.com>
#
# Entry-type module for 'u-boot-spl-pubkey.dtb'
#

import tempfile
import os

from binman.etype.blob_dtb import Entry_blob_dtb

from dtoc import fdt_util

from u_boot_pylib import tools

# This is imported if needed
state = None

# pylint: disable=C0103
class Entry_u_boot_spl_pubkey_dtb(Entry_blob_dtb):
    """U-Boot SPL device tree including public key

    Properties / Entry arguments:
        - key-name-hint: Public key name without extension (.crt).
                    Default is determined by underlying
                    bintool (fdt_add_pubkey), usually 'key'.
        - algo: (Optional) Algorithm used for signing. Default is determined by
                underlying bintool (fdt_add_pubkey), usually 'sha1,rsa2048'
        - required: (Optional) If present this indicates that the key must be
                    verified for the image / configuration to be
                    considered valid

    The following example shows an image containing an SPL which
    is packed together with the dtb. Binman will add a signature
    node to the dtb.

    Example node::

        image {
        ...
            spl {
                filename = "spl.bin"

                u-boot-spl-nodtb {
                };
                u-boot-spl-pubkey-dtb {
                    algo = "sha384,rsa4096";
                    required = "conf";
                    key-name-hint = "dev";
                };
            };
        ...
        }
    """

    def __init__(self, section, etype, node):
        # Put this here to allow entry-docs and help to work without libfdt
        global state
        from binman import state

        super().__init__(section, etype, node)
        self.required_props = ['key-name-hint']
        self.fdt_add_pubkey = None
        self._algo = fdt_util.GetString(self._node, 'algo')
        self._required = fdt_util.GetString(self._node, 'required')
        self._key_name_hint = fdt_util.GetString(self._node, 'key-name-hint')

    def ObtainContents(self, fake_size=0):
        """Add public key to SPL dtb

            Add public key which is pointed out by
            'key-name-hint' to node 'signature' in the spl-dtb

            This is equivalent to the '-K' option of 'mkimage'

        Args:
            fake_size (int): unused
        """

        # We don't pass fake_size upwards because this is currently
        # not supported by the blob type
        super().ObtainContents()

        with tempfile.NamedTemporaryFile(prefix=os.path.basename(
                                         self.GetFdtEtype()),
                                         dir=tools.get_output_dir())\
                                              as pubkey_tdb:
            tools.write_file(pubkey_tdb.name, self.GetData())
            keyname = tools.get_input_filename(self._key_name_hint + ".crt")
            self.fdt_add_pubkey.run(pubkey_tdb.name,
                                    os.path.dirname(keyname),
                                    self._key_name_hint,
                                    self._required, self._algo)
            dtb = tools.read_file(pubkey_tdb.name)
            self.SetContents(dtb)
            state.UpdateFdtContents(self.GetFdtEtype(), dtb)

        return True

    # pylint: disable=R0201,C0116
    def GetDefaultFilename(self):
        return 'spl/u-boot-spl-pubkey.dtb'

    # pylint: disable=R0201,C0116
    def GetFdtEtype(self):
        return 'u-boot-spl-dtb'

    # pylint: disable=R0201,C0116
    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.fdt_add_pubkey = self.AddBintool(btools, 'fdt_add_pubkey')
