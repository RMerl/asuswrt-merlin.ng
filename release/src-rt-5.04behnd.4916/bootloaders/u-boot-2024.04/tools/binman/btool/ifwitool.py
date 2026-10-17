# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for ifwitool

ifwitool provides a way to package firmware in an Intel Firmware Image (IFWI)
file on some Intel SoCs, e.g. Apolo Lake.

Documentation is not really available so far as I can tell

Source code is at tools/ifwitool.c which is a cleaned-up version of
https://github.com/coreboot/coreboot/blob/master/util/cbfstool/ifwitool.c

Here is the help:

ifwitool: Utility for IFWI manipulation

USAGE:
 /tmp/b/sandbox/tools/ifwitool [-h]
 /tmp/b/sandbox/tools/ifwitool FILE COMMAND [PARAMETERS]

COMMANDs:
 add -f FILE -n NAME [-d -e ENTRY]
 create -f FILE
 delete -n NAME
 extract -f FILE -n NAME [-d -e ENTRY]
 print [-d]
 replace -f FILE -n NAME [-d -e ENTRY]
OPTIONs:
 -f FILE : File to read/write/create/extract
 -d      : Perform directory operation
 -e ENTRY: Name of directory entry to operate on
 -v      : Verbose level
 -h      : Help message
 -n NAME : Name of sub-partition to operate on

NAME should be one of:
SMIP(SMIP)
RBEP(CSE_RBE)
FTPR(CSE_BUP)
UCOD(Microcode)
IBBP(Bootblock)
S_BPDT(S-BPDT)
OBBP(OEM boot block)
NFTP(CSE_MAIN)
ISHP(ISH)
DLMP(CSE_IDLM)
IFP_OVERRIDE(IFP_OVERRIDE)
DEBUG_TOKENS(Debug Tokens)
UFS_PHY(UFS Phy)
UFS_GPP(UFS GPP)
PMCP(PMC firmware)
IUNP(IUNIT)
NVM_CONFIG(NVM Config)
UEP(UEP)
UFS_RATE_B(UFS Rate B Config)
"""

from binman import bintool

class Bintoolifwitool(bintool.Bintool):
    """Handles the 'ifwitool' tool

    This bintool supports running `ifwitool` with some basic parameters as
    neeed by binman. It includes creating a file from a FIT as well as adding,
    replacing, deleting and extracting subparts.

    The tool is built as part of U-Boot, but a binary version can be fetched if
    required.

    ifwitool provides a way to package firmware in an Intel Firmware Image
    (IFWI) file on some Intel SoCs, e.g. Apolo Lake.
    """
    def __init__(self, name):
        super().__init__(name, 'Manipulate Intel IFWI files')

    def create_ifwi(self, intel_fit, ifwi_file):
        """Create a new IFWI file, using an existing Intel FIT binary

        Args:
            intel_fit (str): Filename of exist Intel FIT file
            ifwi_file (str): Output filename to write the new IFWI too

        Returns:
            str: Tool output
        """
        args = [intel_fit, 'create', '-f', ifwi_file]
        return self.run_cmd(*args)

    def delete_subpart(self, ifwi_file, subpart):
        """Delete a subpart within the IFWI file

        Args:
            ifwi_file (str): IFWI filename to update
            subpart (str): Name of subpart to delete, e.g. 'OBBP'

        Returns:
            str: Tool output
        """
        args = [ifwi_file, 'delete', '-n', subpart]
        return self.run_cmd(*args)

    # pylint: disable=R0913
    def add_subpart(self, ifwi_file, subpart, entry_name, infile,
                    replace=False):
        """Add or replace a subpart within the IFWI file

        Args:
            ifwi_file (str): IFWI filename to update
            subpart (str): Name of subpart to add/replace
            entry_nme (str): Name of entry to add/replace
            replace (bool): True to replace the existing entry, False to add a
                new one

        Returns:
            str: Tool output
        """
        args = [
            ifwi_file,
            'replace' if replace else 'add',
            '-n', subpart,
            '-d', '-e', entry_name,
            '-f', infile,
            ]
        return self.run_cmd(*args)

    def extract(self, ifwi_file, subpart, entry_name, outfile):
        """Extract a subpart from the IFWI file

        Args:
            ifwi_file (str): IFWI filename to extract from
            subpart (str): Name of subpart to extract
            entry_nme (str): Name of entry to extract

        Returns:
            str: Tool output
        """
        args = [
            ifwi_file,
            'extract',
            '-n', subpart,
            '-d', '-e', entry_name,
            '-f', outfile,
            ]
        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for ifwitool

        This installs ifwitool using a binary download.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched, None if a method other than FETCH_BIN
            was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        fname, tmpdir = self.fetch_from_drive(
            '18JDghOxlt2Hcc5jv51O1t6uNVHQ0XKJS')
        return fname, tmpdir
