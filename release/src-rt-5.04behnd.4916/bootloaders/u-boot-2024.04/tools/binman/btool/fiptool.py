# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for fiptool

fiptool provides a way to package firmware in an ARM Trusted Firmware Firmware
Image Package (ATF FIP) format. It is used with Trusted Firmware A, for example.

Documentation is at:
https://trustedfirmware-a.readthedocs.io/en/latest/getting_started/tools-build.html?highlight=fiptool#building-and-using-the-fip-tool

Source code is at:
https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git

Here is the help:

usage: fiptool [--verbose] <command> [<args>]
Global options supported:
  --verbose	Enable verbose output for all commands.

Commands supported:
  info		List images contained in FIP.
  create	Create a new FIP with the given images.
  update	Update an existing FIP with the given images.
  unpack	Unpack images from FIP.
  remove	Remove images from FIP.
  version	Show fiptool version.
  help		Show help for given command.

"""

from binman import bintool

class Bintoolfiptool(bintool.Bintool):
    """Image generation for ARM Trusted Firmware

    This bintool supports running `fiptool` with some basic parameters as
    neeed by binman.

    It also supports build fiptool from source.

    fiptool provides a way to package firmware in an ARM Trusted Firmware
    Firmware Image Package (ATF FIP) format. It is used with Trusted Firmware A,
    for example.

    See `TF-A FIP tool documentation`_ for more information.

    .. _`TF-A FIP tool documentation`:
        https://trustedfirmware-a.readthedocs.io/en/latest/getting_started/tools-build.html?highlight=fiptool#building-and-using-the-fip-tool
    """
    def __init__(self, name):
        super().__init__(name, 'Manipulate ATF FIP files', r'^(.*)$', 'version')

    def info(self, fname):
        """Get info on a FIP image

        Args:
            fname (str): Filename to check

        Returns:
            str: Tool output
        """
        args = ['info', fname]
        return self.run_cmd(*args)

    # pylint: disable=R0913
    def create_new(self, fname, align, plat_toc_flags, fwu, tb_fw, blob_uuid,
                   blob_file):
        """Create a new FIP

        Args:
            fname (str): Filename to write to
            align (int): Alignment to use for entries
            plat_toc_flags (int): Flags to use for the TOC header
            fwu (str): Filename for the fwu entry
            tb_fw (str): Filename for the tb_fw entry
            blob_uuid (str): UUID for the blob entry
            blob_file (str): Filename for the blob entry

        Returns:
            str: Tool output
        """
        args = [
            'create',
            '--align', f'{align:x}',
            '--plat-toc-flags', f'{plat_toc_flags:#x}',
            '--fwu', fwu,
            '--tb-fw', tb_fw,
            '--blob', f'uuid={blob_uuid},file={blob_file}',
            fname]
        return self.run_cmd(*args)

    def create_bad(self):
        """Run fiptool with invalid arguments"""
        args = ['create', '--fred']
        return self.run_cmd_result(*args)

    def fetch(self, method):
        """Fetch handler for fiptool

        This builds the tool from source

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        if method != bintool.FETCH_BUILD:
            return None
        result = self.build_from_git(
            'https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git',
            ['fiptool'],
            'tools/fiptool/fiptool')
        return result
