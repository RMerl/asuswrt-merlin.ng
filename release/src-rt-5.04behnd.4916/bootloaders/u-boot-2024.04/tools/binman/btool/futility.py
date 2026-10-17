# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for futility

futility (flash utility) is a tool for working with Chromium OS flash images.
This implements just the features used by Binman.

Documentation is at:
   https://chromium.googlesource.com/chromiumos/platform/vboot/+/refs/heads/main/_vboot_reference/README

Source code:
   https://chromium.googlesource.com/chromiumos/platform/vboot/+/refs/heads/master/_vboot_reference/futility

Here is the help:
Usage: futility [options] COMMAND [args...]

This is the unified firmware utility, which will eventually replace
most of the distinct verified boot tools formerly produced by the
vboot_reference package.

When symlinked under the name of one of those previous tools, it should
fully implement the original behavior. It can also be invoked directly
as futility, followed by the original name as the first argument.

Global options:

  --vb1        Use only vboot v1.0 binary formats
  --vb21       Use only vboot v2.1 binary formats
  --debug      Be noisy about what's going on

The following commands are built-in:

  bdb                  Common boot flow utility
  create               Create a keypair from an RSA .pem file
  dump_fmap            Display FMAP contents from a firmware image
  dump_kernel_config   Prints the kernel command line
  gbb                  Manipulate the Google Binary Block (GBB)
  gbb_utility          Legacy name for `gbb` command
  help                 Show a bit of help (you're looking at it)
  load_fmap            Replace the contents of specified FMAP areas
  pcr                  Simulate a TPM PCR extension operation
  show                 Display the content of various binary components
  sign                 Sign / resign various binary components
  update               Update system firmware
  validate_rec_mrc     Validates content of Recovery MRC cache
  vbutil_firmware      Verified boot firmware utility
  vbutil_kernel        Creates, signs, and verifies the kernel partition
  vbutil_key           Wraps RSA keys with vboot headers
  vbutil_keyblock      Creates, signs, and verifies a keyblock
  verify               Verify the signatures of various binary components
  version              Show the futility source revision and build date
"""

from binman import bintool

class Bintoolfutility(bintool.Bintool):
    """Handles the 'futility' tool

    futility (flash utility) is a tool for working with Chromium OS flash
    images. This Bintool implements just the features used by Binman, related to
    GBB creation and firmware signing.

    A binary version of the tool can be fetched.

    See `Chromium OS vboot documentation`_ for more information.

    .. _`Chromium OS vboot documentation`:
        https://chromium.googlesource.com/chromiumos/platform/vboot/+/refs/heads/main/_vboot_reference/README
    """
    def __init__(self, name):
        super().__init__(name, 'Chromium OS firmware utility', r'^(.*)$', 'version')

    def gbb_create(self, fname, sizes):
        """Create a new Google Binary Block

        Args:
            fname (str): Filename to write to
            sizes (list of int): Sizes of each regions:
               hwid_size, rootkey_size, bmpfv_size, recoverykey_size

        Returns:
            str: Tool output
        """
        args = [
            'gbb_utility',
            '-c',
            ','.join(['%#x' % size for size in sizes]),
            fname
            ]
        return self.run_cmd(*args)

    # pylint: disable=R0913
    def gbb_set(self, fname, hwid, rootkey, recoverykey, flags, bmpfv):
        """Set the parameters in a Google Binary Block

        Args:
            fname (str): Filename to update
            hwid (str): Hardware ID to use
            rootkey (str): Filename of root key, e.g. 'root_key.vbpubk'
            recoverykey (str): Filename of recovery key,
                e.g. 'recovery_key.vbpubk'
            flags (int): GBB flags to use
            bmpfv (str): Filename of firmware bitmaps (bmpblk file)

        Returns:
            str: Tool output
        """
        args = ['gbb_utility',
            '-s',
            f'--hwid={hwid}',
            f'--rootkey={rootkey}',
            f'--recoverykey={recoverykey}',
            f'--flags={flags}',
            f'--bmpfv={bmpfv}',
            fname
            ]
        return self.run_cmd(*args)

    def sign_firmware(self, vblock, keyblock, signprivate, version, firmware,
                      kernelkey, flags):
        """Sign firmware to create a vblock file

        Args:
            vblock (str): Filename to write the vblock too
            keyblock (str): Filename of keyblock file
            signprivate (str): Filename of private key
            version (int): Version number
            firmware (str): Filename of firmware binary to sign
            kernelkey (str): Filename of kernel key
            flags (int): Preamble flags

        Returns:
            str: Tool output
        """
        args = [
            'vbutil_firmware',
            '--vblock', vblock,
            '--keyblock', keyblock,
            '--signprivate', signprivate,
            '--version', version,
            '--fv', firmware,
            '--kernelkey', kernelkey,
            '--flags', flags
            ]
        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for futility

        This installs futility using a binary download.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched, None if a method other than FETCH_BIN
            was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BUILD:
            return None

        # The Chromium OS repo is here:
        # https://chromium.googlesource.com/chromiumos/platform/vboot_reference/
        #
        # Unfortunately this requires logging in and obtaining a line for the
        # .gitcookies file. So use a mirror instead.
        result = self.build_from_git(
            'https://github.com/sjg20/vboot_reference.git',
            ['all'],
            'build/futility/futility',
            flags=['USE_FLASHROM=0'])
        return result
